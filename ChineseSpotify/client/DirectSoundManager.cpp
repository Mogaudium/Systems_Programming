// This was my starting point, later I developed a new DLL project "SP_DLL"

#include "DirectSoundManager.h"
#include <stdexcept>

DirectSoundManager::DirectSoundManager(HWND hwnd) :
    directSound(nullptr), primaryBuffer(nullptr), secondaryBuffer(nullptr), wfFormat({})
{
    InitializeDirectSound(hwnd);
}

DirectSoundManager::~DirectSoundManager() {
    if (secondaryBuffer) {
        secondaryBuffer->Release();
    }
    if (primaryBuffer) {
        primaryBuffer->Release();
    }
    if (directSound) {
        directSound->Release();
    }
}

void DirectSoundManager::InitializeDirectSound(HWND hwnd) {
    CreateDirectSoundObject();
    SetCooperativeLevel(hwnd);
    CreatePrimaryBuffer();
    CreateSecondaryBuffer();
}

LPDIRECTSOUNDBUFFER DirectSoundManager::GetSecondaryBuffer() {
    return secondaryBuffer;
}

void DirectSoundManager::CreateDirectSoundObject() {
    if (DirectSoundCreate8(NULL, &directSound, NULL) != DS_OK) {
        throw std::runtime_error("DirectSound initialization failed");
    }
}

void DirectSoundManager::SetCooperativeLevel(HWND hwnd) {
    if (directSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY) != DS_OK) {
        throw std::runtime_error("SetCooperativeLevel failed");
    }
}

void DirectSoundManager::CreatePrimaryBuffer() {
    DSBUFFERDESC bufferDesc = {};
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if (directSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, NULL) != DS_OK) {
        throw std::runtime_error("Primary buffer creation failed");
    }
}

void DirectSoundManager::CreateSecondaryBuffer() {
    ZeroMemory(&wfFormat, sizeof(WAVEFORMATEX));
    wfFormat.wFormatTag = WAVE_FORMAT_PCM;
    wfFormat.nChannels = 2;
    wfFormat.nSamplesPerSec = 44100;
    wfFormat.wBitsPerSample = 16;
    wfFormat.nBlockAlign = (wfFormat.wBitsPerSample / 8) * wfFormat.nChannels;
    wfFormat.nAvgBytesPerSec = wfFormat.nSamplesPerSec * wfFormat.nBlockAlign;

    DSBUFFERDESC bufferDesc = {};
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS;
    bufferDesc.dwBufferBytes = wfFormat.nAvgBytesPerSec * 4;
    bufferDesc.lpwfxFormat = &wfFormat;

    if (directSound->CreateSoundBuffer(&bufferDesc, &secondaryBuffer, NULL) != DS_OK) {
        throw std::runtime_error("Secondary buffer creation failed");
    }
}
