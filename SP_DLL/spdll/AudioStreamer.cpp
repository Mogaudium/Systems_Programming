#include "pch.h"
#include <stdexcept>
#include <Mmreg.h>
#include <thread>
#include <algorithm>

#include "AudioStreamer.h"

// Constructor: Initializes the AudioStreamer with provided sockets and buffers
AudioStreamer::AudioStreamer(SOCKET clientSocket, LPDIRECTSOUNDBUFFER secondaryBuffer1, LPDIRECTSOUNDBUFFER secondaryBuffer2) :
    clientSocket(clientSocket), secondaryBuffer1(secondaryBuffer1), secondaryBuffer2(secondaryBuffer2), currentBufferIndex(0) {}

// Public function to start streaming audio data in a separate thread
void AudioStreamer::StartStreaming() {
    audioThread = std::thread(&AudioStreamer::StreamAudioData, this);
}

void AudioStreamer::Pause(bool pause) {
    paused = pause;

    LPDIRECTSOUNDBUFFER& currentBuffer = (currentBufferIndex == 0) ? secondaryBuffer1 : secondaryBuffer2;

    if (pause) {
        currentBuffer->Stop();
    }
    else {
        // Resume playback if there is data in the buffer
        currentBuffer->Play(0, 0, 0);
    }
}

void AudioStreamer::SetVolume(float newVolume) {
    // Ensure the volume is within the valid range (0.0 to 1.0)
    if (newVolume < 0.0f)
        volume = 0.0f;
    else if (newVolume > 1.0f)
        volume = 1.0f;
    else
        volume = newVolume;

    // Convert linear volume to decibel scale
    LONG volumeDB;
    if (volume == 0.0f) {
        volumeDB = DSBVOLUME_MIN;  // Minimum volume (silence)
    }
    else {
        volumeDB = static_cast<LONG>(20 * log10(volume) * (DSBVOLUME_MAX - DSBVOLUME_MIN) / 100);
    }

    // Apply volume to both secondary buffers
    secondaryBuffer1->SetVolume(volumeDB);
    secondaryBuffer2->SetVolume(volumeDB);
}

// Destructor: Waits for the audio thread to finish execution before cleaning up resources
AudioStreamer::~AudioStreamer() {
    if (audioThread.joinable()) {
        audioThread.join(); 
    }
}

// Streams audio data from the server and plays it using DirectSound
void AudioStreamer::StreamAudioData() {
    // Audio format parameters
    int sampleRate = 44100;
    int bitsPerSample = 16;
    int numChannels = 2;
    float bufferDuration = 1.5;

    // Calculate buffer size based on audio format
    int bufferSize = static_cast<int>(sampleRate * numChannels * (bitsPerSample / static_cast<float>(8)) * bufferDuration);

    // Vector to hold audio data chunks
    std::vector<char> audioChunk(bufferSize);

    // Variables to track total file size and received bytes
    std::streamsize totalFileSize = 0, totalBytesReceived = 0;

    // Receive total file size from the server
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalFileSize), sizeof(totalFileSize), 0);
    if (bytesReceived <= 0) {
        std::cerr << "Error receiving file size, error code: " << WSAGetLastError() << std::endl;
        return;
    }

    // Loop to receive and play audio data chunks
    while (totalBytesReceived < totalFileSize && !paused) {

        if (paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep while paused
            continue;
        }

        // Determine the current secondary buffer
        LPDIRECTSOUNDBUFFER& currentBuffer = (currentBufferIndex == 0) ? secondaryBuffer1 : secondaryBuffer2;

        // Receive audio data chunk from the server
        int bytesRead = ReceiveAudioChunk(audioChunk.data(), bufferSize);
        if (bytesRead <= 0) break;

        // Update total received bytes
        totalBytesReceived += bytesRead;


        if (!paused) {

            // Load audio data into the current buffer
            LoadBuffer(currentBuffer, audioChunk.data(), bytesRead);

            // Play the loaded buffer
            PlayBuffer(currentBuffer);
        }

        // Switch to the other buffer
        currentBufferIndex = 1 - currentBufferIndex;

        // Wait for the current buffer to finish playing
        WaitForBuffer(currentBuffer);
    }

    // Check if the entire audio file was received and played
    if (totalBytesReceived == totalFileSize) {
        std::cout << "Received and played entire audio file." << std::endl;
    }
    else {
        std::cerr << "Received incomplete file." << std::endl;
    }
}

// Loads audio data into a DirectSound buffer
void AudioStreamer::LoadBuffer(LPDIRECTSOUNDBUFFER buffer, char* data, int size) {
    // Variables for DirectSound buffer locking
    VOID* bufferPtr1 = nullptr;
    VOID* bufferPtr2 = nullptr;
    DWORD bufferSize1 = 0;
    DWORD bufferSize2 = 0;

    // Lock the DirectSound buffer to write audio data
    HRESULT hr = buffer->Lock(0, size, &bufferPtr1, &bufferSize1, &bufferPtr2, &bufferSize2, 0);
    if (FAILED(hr)) {
        std::cerr << "Failed to lock buffer: " << hr << std::endl;
        return;
    }

    // Copy audio data to the buffer
    memcpy(bufferPtr1, data, bufferSize1);
    if (bufferPtr2 != NULL) {
        memcpy(bufferPtr2, data + bufferSize1, bufferSize2);
    }

    // Unlock the buffer to allow playback
    buffer->Unlock(bufferPtr1, bufferSize1, bufferPtr2, bufferSize2);
}

// Plays the audio data loaded into a DirectSound buffer
void AudioStreamer::PlayBuffer(LPDIRECTSOUNDBUFFER buffer) {
    // Start playback of the DirectSound buffer
    HRESULT hr = buffer->Play(0, 0, 0);
    if (FAILED(hr)) {
        std::cerr << "Failed to start playback: " << hr << std::endl;
    }
}

// Waits for a DirectSound buffer to finish playing
void AudioStreamer::WaitForBuffer(LPDIRECTSOUNDBUFFER buffer) {
    // Variable to store buffer status
    DWORD status;

    // Wait until the buffer finishes playing
    while (true) {
        buffer->GetStatus(&status);
        if (!(status & DSBSTATUS_PLAYING)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Sleep to prevent busy waiting
    }
}

// Private function: Receives an audio data chunk from the server
int AudioStreamer::ReceiveAudioChunk(char* buffer, int bufferSize) {
    int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesRead == SOCKET_ERROR) {
        std::cerr << "Network error: " << WSAGetLastError() << std::endl;
        return -1; 
    }
    return bytesRead;
}
