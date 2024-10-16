// This was my starting point, later I developed a new DLL project "SP_DLL"

#include "AudioStreamer.h"
#include <stdexcept>

AudioStreamer::AudioStreamer(SOCKET clientSocket, LPDIRECTSOUNDBUFFER secondaryBuffer)
    : clientSocket(clientSocket), secondaryBuffer(secondaryBuffer), isStreaming(false) {
    playbackThread = std::thread(&AudioStreamer::PlaybackThread, this);
}


AudioStreamer::~AudioStreamer() {
    isStreaming = false;
    bufferCondVar.notify_one();

    if (playbackThread.joinable()) {
        playbackThread.join();
    }
}

void AudioStreamer::StreamAudioData() {
    const int bufferSize = 44100 * 4; 
    std::vector<char> audioChunk(bufferSize);
    std::streamsize totalFileSize = 0, totalBytesReceived = 0;

    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalFileSize), sizeof(totalFileSize), 0);
    if (bytesReceived <= 0) {
        std::cerr << "Error receiving file size." << std::endl;
        return;
    }

    isStreaming = true;
    while (totalBytesReceived < totalFileSize) {
        int bytesRead = ReceiveAudioChunk(audioChunk.data(), bufferSize);
        if (bytesRead <= 0) break;

        totalBytesReceived += bytesRead;

        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            bufferQueue.push(std::vector<char>(audioChunk.begin(), audioChunk.begin() + bytesRead));
        }
        bufferCondVar.notify_one();
    }

    isStreaming = false;
    bufferCondVar.notify_one(); 
}

void AudioStreamer::PlaybackThread() {
    while (isStreaming || !bufferQueue.empty()) {
        std::unique_lock<std::mutex> lock(bufferMutex);
        bufferCondVar.wait(lock, [this] { return !isStreaming || !bufferQueue.empty(); });

        if (!bufferQueue.empty()) {
            auto& audioChunk = bufferQueue.front();

            VOID* bufferPtr1;
            VOID* bufferPtr2;
            DWORD bufferSize1;
            DWORD bufferSize2;
            HRESULT hr = secondaryBuffer->Lock(0, audioChunk.size(), &bufferPtr1, &bufferSize1, &bufferPtr2, &bufferSize2, 0);
            if (FAILED(hr)) {
                std::cerr << "Failed to lock buffer: " << hr << std::endl;
                break; 
            }

            memcpy(bufferPtr1, audioChunk.data(), bufferSize1);
            if (bufferPtr2 != NULL) {
                memcpy(bufferPtr2, audioChunk.data() + bufferSize1, bufferSize2);
            }

            secondaryBuffer->Unlock(bufferPtr1, bufferSize1, bufferPtr2, bufferSize2);

            hr = secondaryBuffer->Play(0, 0, 0);
            if (FAILED(hr)) {
                std::cerr << "Failed to start playback: " << hr << std::endl;
                break;
            }

            DWORD status;
            do {
                secondaryBuffer->GetStatus(&status);
            } while (status & DSBSTATUS_PLAYING);

            bufferQueue.pop();
        }
    }
}

int AudioStreamer::ReceiveAudioChunk(char* buffer, int bufferSize) {
    int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
    return bytesRead;
}
