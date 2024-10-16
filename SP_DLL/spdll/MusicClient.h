#pragma once
#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <thread>

#include "DirectSoundManager.h"
#include "AudioStreamer.h"

class MusicClient {
private:
    SOCKET clientSocket;  // Socket for communication with the server
    std::unique_ptr<DirectSoundManager> dsManager;  // Manages DirectSound functionality
    std::unique_ptr<AudioStreamer> audioStreamer;   // Manages audio streaming
    std::thread streamThread;  // Thread to manage streaming
    HWND hwnd;  // Window handle for DirectSound initialization

public:
    // Constructor: Initializes the MusicClient with a specified window handle (HWND)
    explicit MusicClient(HWND hwnd);

    // Destructor: Ensures proper cleanup of resources
    ~MusicClient();

    // Connects to the server and prepares the audio streamer
    void ConnectAndPrepareStreamer(const std::string& ipAddress, int port);

    // Receives a list of available audio tracks from the server
    std::vector<std::wstring> ReceiveFileList();

    // Pauses or resumes audio streaming
    void PauseStreaming(bool pause);

    // Sets the initial volume
    float initialVolume; 

    // Sets the volume for audio streaming (0.0 to 1.0)
    void SetVolume(float volume);

    // Sends a request to the server to play a specific audio track
    void RequestAudioTrack(const std::wstring& trackName);

    // Starts streaming audio data
    void StartStreaming();
};
