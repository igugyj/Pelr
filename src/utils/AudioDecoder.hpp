#pragma once

#include <QString>
#include <vector>

struct DecodedAudio
{
    std::vector<float> pcmData;
    unsigned int sampleRate = 0;
    unsigned int channels = 0;
    unsigned long long totalFrameCount = 0;
    bool valid = false;
};

class AudioDecoder
{
public:
    static DecodedAudio decode(const QString &filePath);
};
