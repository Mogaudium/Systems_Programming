#pragma once
#include <windows.h>

// Define macro for export/import 
#ifdef AUDIOCLIENTDLL_EXPORTS
#define AUDIOCLIENTDLL_API __declspec(dllexport)
#else
#define AUDIOCLIENTDLL_API __declspec(dllimport)
#endif

extern "C" {
    // Initializes the MusicClient with a specified window handle (HWND)
    AUDIOCLIENTDLL_API void InitializeClient(HWND hwnd);

    // Connects to the server and prepares the audio streamer
    AUDIOCLIENTDLL_API void ConnectToServer(const char* ipAddress, int port);

    // Retrieves a list of available audio tracks from the server
    AUDIOCLIENTDLL_API const wchar_t* GetFileList(int* count);

    // Sends a request to the server to play a specific audio track
    AUDIOCLIENTDLL_API void RequestAudioTrack(const wchar_t* trackName);

    // Pauses or resumes audio streaming
    AUDIOCLIENTDLL_API void PauseStreaming(bool pause);

    // Sets the volume for audio streaming (0.0 to 1.0)
    AUDIOCLIENTDLL_API void SetVolume(float volume);

    // Starts streaming audio data
    AUDIOCLIENTDLL_API void StartStreaming();
}
