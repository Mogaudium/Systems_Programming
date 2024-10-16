// This was my starting point, later I developed a new DLL project "SP_DLL"

#include <iostream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <dsound.h>
#include <stdexcept>
#include <string>
#include <memory>
#include <sstream>

#include "DirectSoundManager.h"
#include "AudioStreamer.h"

HWND CreateSimpleWindow() {
    const wchar_t* className = L"DirectSoundWindowClass";
    WNDCLASSW wndClass = {};
    wndClass.lpfnWndProc = DefWindowProcW;
    wndClass.hInstance = GetModuleHandle(NULL);
    wndClass.lpszClassName = className;

    if (!RegisterClassW(&wndClass)) {
        throw std::runtime_error("Window registration failed");
    }

    HWND hwnd = CreateWindowExW(
        0,
        className,
        L"DirectSound Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hwnd) {
        throw std::runtime_error("Window creation failed");
    }

    return hwnd;
}

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 54000;
const int MAX_BUFFER_SIZE = 128 * 1024;

class MusicClient {
private:
    SOCKET clientSocket;
    std::unique_ptr<DirectSoundManager> dsManager;
    std::unique_ptr<AudioStreamer> audioStreamer;
    HWND hwnd;

public:
    MusicClient(HWND hwnd) : clientSocket(INVALID_SOCKET), hwnd(hwnd) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }

        dsManager = std::make_unique<DirectSoundManager>(hwnd);
    }

    void ConnectAndPrepareStreamer(const std::string& ipAddress, int port) {
        ConnectToServer(ipAddress, port);
        audioStreamer = std::make_unique<AudioStreamer>(clientSocket, dsManager->GetSecondaryBuffer());
    }

    ~MusicClient() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
        }
        WSACleanup();
    }

    void ConnectToServer(const std::string& ipAddress, int port) {
        std::cout << "Attempting to connect to server " << ipAddress << ":" << port << std::endl;

        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            throw std::runtime_error("Error at socket(): " + std::to_string(WSAGetLastError()));
        }
        else {
            std::cout << "You are connected" << std::endl;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);

        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            throw std::runtime_error("Can't connect to server, error: " + std::to_string(WSAGetLastError()));
        }
    }

    std::wstring ReceiveFileList() {
        char buffer[MAX_BUFFER_SIZE];
        std::string receivedData;
        std::vector<std::wstring> fileList;

        while (true) {
            int bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
            if (bytesReceived <= 0) break; 

            receivedData.append(buffer, bytesReceived);

            if (receivedData.find("END\n") != std::string::npos) {
                break; 
            }
        }

        std::stringstream ss(receivedData);
        std::string line;
        while (std::getline(ss, line)) {
            if (line == "END") break; 
            std::wstring wideFile(line.begin(), line.end());
            fileList.push_back(wideFile);
        }

        for (size_t i = 0; i < fileList.size(); ++i) {
            std::wcout << i + 1 << L": " << fileList[i] << std::endl;
        }

        std::wcout << L"Enter the number of the track you want to play: ";
        int choice;
        std::wcin >> choice;

        if (choice > 0 && choice <= static_cast<int>(fileList.size())) {
            return fileList[choice - 1];
        }
        else {
            std::wcerr << L"Invalid choice. Please try again." << std::endl;
            return L"";
        }
    }

    void RequestAudioTrack(const std::wstring& trackName) {
        std::wstring request = L"PLAY " + trackName + L"\n";
        std::string narrowRequest(request.begin(), request.end());
        send(clientSocket, narrowRequest.c_str(), narrowRequest.length(), 0);
    }

    void StreamAudioData() {
        audioStreamer->StreamAudioData();
    }
};

    int main() {
        try {
            HWND hwnd = CreateSimpleWindow();
            ShowWindow(hwnd, SW_SHOW);

            MusicClient client(hwnd);

            client.ConnectAndPrepareStreamer(SERVER_IP, SERVER_PORT);
            std::wstring selectedFile = client.ReceiveFileList();
            if (!selectedFile.empty()) {
                client.RequestAudioTrack(selectedFile);
                client.StreamAudioData();
            }
        }
        catch (const std::runtime_error& e) {
            std::cerr << "Runtime error: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "An unknown error occurred" << std::endl;
        }

        return 0;
    }