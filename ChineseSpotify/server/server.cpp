// This is my server, it is used to send data

#include <iostream>
#include <fstream>
#include <winsock2.h>   
#include <ws2tcpip.h>   
#include <thread>
#include <vector>
#include <stdexcept>
#include <string>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")  // Link to the Winsock library

const int PORT = 54000;  // Port number for the server
const int MAX_BUFFER_SIZE = 512 * 1024;  // Maximum buffer size for data transfer

// Function to list .wav files in a directory
std::vector<std::wstring> ListWavFiles(const std::wstring& directory) {
    std::vector<std::wstring> fileList;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directory + L"\\*.wav").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            fileList.push_back(findFileData.cFileName);
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }

    // Debug output: Print the list of found .wav files
    for (const auto& file : fileList) {
        std::wcout << L"Found file: " << file << std::endl;
    }

    return fileList;
}

// Function to send data over a socket
void SendData(SOCKET clientSocket, const char* buffer, int bufferSize) {
    int bytesSent = send(clientSocket, buffer, bufferSize, 0);
    if (bytesSent == SOCKET_ERROR) {
        throw std::runtime_error("Error sending data: " + std::to_string(WSAGetLastError()));
    }
}

// Function to send a list of audio files to the client
void SendFileList(SOCKET clientSocket, const std::vector<std::wstring>& fileList) {
    for (const auto& file : fileList) {
        // Convert wstring to string
        std::string narrowFile(file.begin(), file.end());
        std::string message = narrowFile + "\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    // Indicate end of the file list
    std::string endOfListMarker = "END\n";
    send(clientSocket, endOfListMarker.c_str(), endOfListMarker.length(), 0);
}

// Function to send an audio file to the client
void SendAudioFile(SOCKET clientSocket, const std::string& trackName) {
    std::ifstream audioFile("C:\\Users\\ipman\\source\\repos\\SP_100610788\\ChineseSpotify\\x64\\Debug\\" + trackName, std::ios::binary | std::ios::ate);
    if (!audioFile.is_open()) {
        throw std::runtime_error("Failed to open audio file: " + trackName);
    }

    std::streamsize fileSize = audioFile.tellg();
    audioFile.seekg(0, std::ios::beg);
    SendData(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));

    std::vector<char> buffer(MAX_BUFFER_SIZE);
    while (!audioFile.eof()) {
        audioFile.read(buffer.data(), buffer.size());
        size_t bytesRead = audioFile.gcount();
        SendData(clientSocket, buffer.data(), bytesRead);
    }

    audioFile.close();
}

// Function to handle a client connection
void HandleClient(SOCKET clientSocket) {
    try {
        // Specify the directory path
        std::wstring directoryPath = L"C:\\Users\\ipman\\source\\repos\\SP_100610788\\ChineseSpotify\\x64\\Debug\\";
        // Get the list of .wav files
        std::vector<std::wstring> fileList = ListWavFiles(directoryPath);
        SendFileList(clientSocket, fileList);

        char recvbuf[MAX_BUFFER_SIZE];
        int recvbuflen = MAX_BUFFER_SIZE;

        int iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::string request(recvbuf, iResult);

            // Check if the request starts with "PLAY "
            if (request.rfind("PLAY ", 0) == 0) {
                std::string trackName = request.substr(5); // Extract file name after "PLAY "
                trackName.erase(std::remove(trackName.begin(), trackName.end(), '\n'), trackName.end()); // Remove newline character
                SendAudioFile(clientSocket, trackName);
            }
            else {
                std::cerr << "Invalid request received: " << request << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in HandleClient: " << e.what() << std::endl;
    }

    closesocket(clientSocket);  // Close the client socket
}

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);  // Initialize Winsock library
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // Create a socket
    if (listenSocket == INVALID_SOCKET) {
        throw std::runtime_error("Error creating socket: " + std::to_string(WSAGetLastError()));
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);  // Specify the port number
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        throw std::runtime_error("Bind failed with error: " + std::to_string(WSAGetLastError()));
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        throw std::runtime_error("Listen failed with error: " + std::to_string(WSAGetLastError()));
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);  // Accept incoming client connections
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::thread clientThread(HandleClient, clientSocket);  // Create a new thread to handle the client
        clientThread.detach();  // Detach the thread to run independently
    }

    closesocket(listenSocket);  // Close the listening socket
    WSACleanup();  // Clean up Winsock resources

    return 0;
}
