// This was my starting point, later I developed a new DLL project "SP_DLL"

#pragma once
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <dsound.h>
#include <thread>

class AudioStreamer {
public:
    AudioStreamer(SOCKET clientSocket, LPDIRECTSOUNDBUFFER secondaryBuffer);
    ~AudioStreamer();
    void StreamAudioData();

private:
    SOCKET clientSocket;
    LPDIRECTSOUNDBUFFER secondaryBuffer;
    int ReceiveAudioChunk(char* buffer, int bufferSize);

    std::queue<std::vector<char>> bufferQueue;
    std::mutex bufferMutex;
    std::condition_variable bufferCondVar;
    bool isStreaming;

    void PlaybackThread();
    std::thread playbackThread;
};
