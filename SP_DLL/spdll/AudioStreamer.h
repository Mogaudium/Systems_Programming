#pragma once
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <dsound.h>
#include <Mmreg.h>
#include <thread>
#include <algorithm>

class AudioStreamer {
public:
    // Constructor: Initialize with two DirectSound secondary buffers
    AudioStreamer(SOCKET clientSocket, LPDIRECTSOUNDBUFFER secondaryBuffer1, LPDIRECTSOUNDBUFFER secondaryBuffer2);

    ~AudioStreamer();  // Destructor

    void StartStreaming();  // Starts streaming in a separate thread
    void StreamAudioData();  //Starts streaming 

    void Pause(bool pause); // Pause or resume audio playback
    void SetVolume(float volume); // Set the volume of the audio stream (0.0 to 1.0)

private:
    SOCKET clientSocket;  // Network socket for audio data
    LPDIRECTSOUNDBUFFER secondaryBuffer1;  // First secondary DirectSound buffer
    LPDIRECTSOUNDBUFFER secondaryBuffer2;  // Second secondary DirectSound buffer
    int currentBufferIndex = 0;            // Index to track the current buffer
    std::thread audioThread;               // Thread for audio streaming

    bool paused = false; // Flag to indicate whether playback is paused
    float volume = 1.0f; // Initial volume (full volume)

    // Private function: Loads data into a DirectSound buffer
    void LoadBuffer(LPDIRECTSOUNDBUFFER buffer, char* data, int size);

    // Private function: Plays the given DirectSound buffer
    void PlayBuffer(LPDIRECTSOUNDBUFFER buffer);

    // Private function: Waits for the given buffer to finish playing
    void WaitForBuffer(LPDIRECTSOUNDBUFFER buffer);

    // Private function: Receives a chunk of audio data from the network
    int ReceiveAudioChunk(char* buffer, int bufferSize);
};
