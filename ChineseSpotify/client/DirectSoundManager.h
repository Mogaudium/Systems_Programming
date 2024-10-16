// This was my starting point, later I developed a new DLL project "SP_DLL"

#pragma once
#include <windows.h>
#include <dsound.h>

class DirectSoundManager {
public:
    DirectSoundManager(HWND hwnd);
    ~DirectSoundManager();
    void InitializeDirectSound(HWND hwnd);
    LPDIRECTSOUNDBUFFER GetSecondaryBuffer();

private:
    LPDIRECTSOUND8 directSound;
    LPDIRECTSOUNDBUFFER primaryBuffer;
    LPDIRECTSOUNDBUFFER secondaryBuffer;
    WAVEFORMATEX wfFormat;

    void CreateDirectSoundObject();
    void SetCooperativeLevel(HWND hwnd);
    void CreatePrimaryBuffer();
    void CreateSecondaryBuffer();
};
