#include "pch.h"
#include <memory>
#include <vector>

#include "AudioClientDLL.h"
#include "MusicClient.h"

// Global instance of MusicClient
std::unique_ptr<MusicClient> g_client;

extern "C" {

    // Initializes the MusicClient with a specified window handle (HWND)
    __declspec(dllexport) void InitializeClient(HWND hwnd) {
        try {
            g_client = std::make_unique<MusicClient>(hwnd);
        }
        catch (const std::exception& e) {
        }
    }

    // Connects to the server and prepares the audio streamer
    __declspec(dllexport) void ConnectToServer(const char* ipAddress, int port) {
        try {
            if (g_client) {
                g_client->ConnectAndPrepareStreamer(ipAddress, port);
            }
        }
        catch (const std::exception& e) {
        }
    }

    // Retrieves a list of available audio tracks from the server
    __declspec(dllexport) const wchar_t* GetFileList(int* count) {
        static std::wstring concatenatedFileList;
        try {
            if (g_client) {
                auto fileList = g_client->ReceiveFileList();
                concatenatedFileList.clear();

                for (const auto& file : fileList) {
                    concatenatedFileList += file + L';';  
                }

                // Set the count 
                if (count) {
                    *count = static_cast<int>(fileList.size());
                }

                return concatenatedFileList.c_str();
            }
        }
        catch (const std::exception& e) {
            // Handle exceptions
            if (count) {
                *count = 0;
            }
            return L"Error";  // Return a simple error indicator
        }

        return nullptr;
    }

    // Pauses or resumes audio streaming
    __declspec(dllexport) void PauseStreaming(bool pause) {
        try {
            if (g_client) {
                g_client->PauseStreaming(pause);
            }
        }
        catch (const std::exception& e) {
        }
    }

    // Sets the volume for audio streaming (0.0 to 1.0)
    __declspec(dllexport) void SetVolume(float volume) {
        try {
            if (g_client) {
                g_client->SetVolume(volume);
            }
        }
        catch (const std::exception& e) {
        }
    }

    // Sends a request to the server to play a specific audio track
    __declspec(dllexport) void RequestAudioTrack(const wchar_t* trackName) {
        try {
            if (g_client) {
                g_client->RequestAudioTrack(trackName);
            }
        }
        catch (const std::exception& e) {
        }
    }

    // Starts streaming audio data
    __declspec(dllexport) void StartStreaming() {
        try {
            if (g_client) {
                g_client->StartStreaming();
            }
        }
        catch (const std::exception& e) {
        }
    }

}
