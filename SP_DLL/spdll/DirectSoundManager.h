#pragma once
#include <windows.h>
#include <dsound.h>
#include <Mmreg.h>

class DirectSoundManager {
public:
    // Constructor: Initializes DirectSoundManager
    DirectSoundManager(HWND hwnd);

    // Destructor: Releases resources when the object is destroyed
    ~DirectSoundManager();

    // Initializes the DirectSoundManager and related components
    void InitializeDirectSound(HWND hwnd);

    // Retrieves a specific secondary buffer (index can be 0 or 1)
    LPDIRECTSOUNDBUFFER GetSecondaryBuffer(int index);

private:
    LPDIRECTSOUND8 directSound;               // DirectSound object
    LPDIRECTSOUNDBUFFER primaryBuffer;        // Primary buffer (not used for streaming)
    LPDIRECTSOUNDBUFFER secondaryBuffers[2];  // Array to hold two secondary buffers
    WAVEFORMATEX wfFormat;                    // Audio format settings

    // Creates the DirectSound object
    void CreateDirectSoundObject();

    // Sets the cooperative level to prioritize the application's access to the sound device
    void SetCooperativeLevel(HWND hwnd);

    // Creates the primary buffer (not used for streaming)
    void CreatePrimaryBuffer();

    // Creates two secondary buffers for streaming audio
    void CreateSecondaryBuffers();
};
