#include "pch.h"
#include <stdexcept>
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>

#include "AudioStreamer.h"
#include "DirectSoundManager.h"
#include "MusicClient.h"

const int MAX_BUFFER_SIZE = 128 * 1024; // Maximum buffer size for data transfer

// Constructor: Initializes the MusicClient with a specified window handle (HWND)
MusicClient::MusicClient(HWND hwnd) : clientSocket(INVALID_SOCKET), hwnd(hwnd), initialVolume(1.0f) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    // Create a DirectSoundManager instance for managing audio
    dsManager = std::make_unique<DirectSoundManager>(hwnd);
}

// Connects to the server and prepares the audio streamer
void MusicClient::ConnectAndPrepareStreamer(const std::string& ipAddress, int port) {
    // Create a socket to connect to the server
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        throw std::runtime_error("Error at socket(): " + std::to_string(WSAGetLastError()));
    }

    // Set up server address information
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);

    // Connect to the server
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        throw std::runtime_error("Can't connect to server, error: " + std::to_string(WSAGetLastError()));
    }

    // Prepare the audio streamer with two secondary buffers
    audioStreamer = std::make_unique<AudioStreamer>(clientSocket, dsManager->GetSecondaryBuffer(0), dsManager->GetSecondaryBuffer(1));

    audioStreamer->SetVolume(initialVolume);

}

// Receives a list of file names from the server
std::vector<std::wstring> MusicClient::ReceiveFileList() {
    char buffer[MAX_BUFFER_SIZE];
    std::string receivedData;
    std::vector<std::wstring> fileList;

    // Receive data until the end-of-list marker is found
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break; // Error or server closed connection

        // Append received data to the buffer
        receivedData.append(buffer, bytesReceived);

        // Check if the end-of-list marker is received
        if (receivedData.find("END\n") != std::string::npos) {
            break; // End of list marker found
        }
    }

    // Process received data to extract file names
    std::stringstream ss(receivedData);
    std::string line;
    while (std::getline(ss, line)) {
        if (line == "END") break; // End of list marker
        std::wstring wideFile(line.begin(), line.end());
        fileList.push_back(wideFile);
    }

    return fileList;
}

// Pauses the playback
void MusicClient::PauseStreaming(bool pause) {
    if (audioStreamer) {
        audioStreamer->Pause(pause);
    }
}

// Sets the volume
void MusicClient::SetVolume(float volume) {
    if (audioStreamer) {
        audioStreamer->SetVolume(volume);
    }
}

// Sends a request to play a specific audio track to the server
void MusicClient::RequestAudioTrack(const std::wstring& trackName) {
    // Construct the request message
    std::wstring request = L"PLAY " + trackName + L"\n";
    std::string narrowRequest(request.begin(), request.end());

    // Send the request to the server
    send(clientSocket, narrowRequest.c_str(), narrowRequest.length(), 0);
}

// Starts streaming audio in a separate thread
void MusicClient::StartStreaming() {
    // Start streaming in a separate thread and ensure it's managed properly
    if (streamThread.joinable()) {
        streamThread.join();  // Join the previous thread if it's running
    }
    streamThread = std::thread(&AudioStreamer::StartStreaming, audioStreamer.get());
}

// Destructor: Ensures proper cleanup of resources
MusicClient::~MusicClient() {
    // Ensure the streaming thread finishes before destruction
    if (streamThread.joinable()) {
        streamThread.join();
    }

    // Close the socket and clean up Winsock
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
    }
    WSACleanup();
}
