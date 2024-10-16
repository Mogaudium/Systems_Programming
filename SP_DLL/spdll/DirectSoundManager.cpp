#include "pch.h"
#include <stdexcept>

#include "DirectSoundManager.h"

// Constructor: Initializes member variables and sets up DirectSound
DirectSoundManager::DirectSoundManager(HWND hwnd) :
    directSound(nullptr), primaryBuffer(nullptr), wfFormat({})
{
    ZeroMemory(&secondaryBuffers, sizeof(secondaryBuffers));  // Initialize the secondary buffer array
    InitializeDirectSound(hwnd);
}

// Destructor: Releases resources when the object is destroyed
DirectSoundManager::~DirectSoundManager() {
    for (auto& buffer : secondaryBuffers) {  // Loop through secondary buffers and release them
        if (buffer) {
            buffer->Release();
        }
    }
    if (primaryBuffer) {
        primaryBuffer->Release();  // Release the primary buffer
    }
    if (directSound) {
        directSound->Release();  // Release the DirectSound object
    }
}

// Initializes the DirectSound object and related components
void DirectSoundManager::InitializeDirectSound(HWND hwnd) {
    CreateDirectSoundObject();
    SetCooperativeLevel(hwnd);
    CreatePrimaryBuffer();
    CreateSecondaryBuffers();
}

// Creates the DirectSound object
void DirectSoundManager::CreateDirectSoundObject() {
    if (DirectSoundCreate8(nullptr, &directSound, nullptr) != DS_OK) {
        throw std::runtime_error("DirectSound initialization failed");
    }
}

// Sets the cooperative level to prioritize the application's access to the sound device
void DirectSoundManager::SetCooperativeLevel(HWND hwnd) {
    if (directSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY) != DS_OK) {
        throw std::runtime_error("SetCooperativeLevel failed");
    }
}

// Creates the primary buffer (not used for streaming audio)
void DirectSoundManager::CreatePrimaryBuffer() {
    DSBUFFERDESC bufferDesc{};

    ZeroMemory(&wfFormat, sizeof(WAVEFORMATEX));
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    bufferDesc.dwBufferBytes = 0;  // Primary buffer has 0 size
    bufferDesc.lpwfxFormat = nullptr;

    if (directSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, nullptr) != DS_OK) {
        throw std::runtime_error("Primary buffer creation failed");
    }
}

// Creates secondary buffers used for streaming audio
void DirectSoundManager::CreateSecondaryBuffers() {
    DSBUFFERDESC bufferDesc{};

    // Set up audio format parameters
    ZeroMemory(&wfFormat, sizeof(WAVEFORMATEX));
    wfFormat.wFormatTag = WAVE_FORMAT_PCM;
    wfFormat.nChannels = 2;
    wfFormat.nSamplesPerSec = 44100;
    wfFormat.wBitsPerSample = 16;
    wfFormat.nBlockAlign = wfFormat.wBitsPerSample / 8 * wfFormat.nChannels;
    wfFormat.nAvgBytesPerSec = wfFormat.nSamplesPerSec * wfFormat.nBlockAlign;

    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS;
    bufferDesc.dwBufferBytes = 1.5 * wfFormat.nAvgBytesPerSec;  // Buffer size is 1.5 times the average bytes per second
    bufferDesc.lpwfxFormat = &wfFormat;

    // Create secondary buffers
    for (auto& buffer : secondaryBuffers) {
        if (directSound->CreateSoundBuffer(&bufferDesc, &buffer, nullptr) != DS_OK) {
            throw std::runtime_error("Secondary buffer creation failed");
        }
    }
}

// Retrieves a secondary buffer based on the index (0 or 1)
LPDIRECTSOUNDBUFFER DirectSoundManager::GetSecondaryBuffer(int index) {
    if (index < 0 || index >= 2) {
        throw std::out_of_range("Buffer index out of range");
    }
    return secondaryBuffers[index];
}
