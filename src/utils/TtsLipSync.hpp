#pragma once
#include "AudioDecoder.hpp"
#include <cmath>
#include <algorithm>

class TtsLipSync
{
public:
    void start(const QString &filePath)
    {
        auto audio = AudioDecoder::decode(filePath);
        if (!audio.valid) return;
        _duration = float(audio.totalFrameCount) / audio.sampleRate;
        _elapsed = 0.0f;
        _value = 0.15f;
    }

    void stop()
    {
        _duration = 0.0f;
        _elapsed = 0.0f;
        _value = 0.0f;
    }

    bool isActive() const { return _duration > 0.0f; }

    bool tick(float deltaSeconds, float &out)
    {
        if (_duration <= 0.0f)
            return false;

        _elapsed += deltaSeconds;
        if (_elapsed >= _duration)
        {
            stop();
            return false;
        }

        float t = _elapsed;
        float n = 0.0f;
        n += noise(t, 8.0f, 0.0f) * 0.18f;
        n += noise(t, 3.2f, 1.0f) * 0.12f;
        n += noise(t, 1.1f, 2.0f) * 0.08f;

        float raw = std::clamp(0.35f + n, 0.15f, 0.65f);

        float fadeIn  = std::min(1.0f, _elapsed / 0.10f);
        float fadeOut = std::min(1.0f, (_duration - _elapsed) / 0.20f);
        raw *= fadeIn * fadeOut;

        _value += 0.4f * (raw - _value);
        out = _value;
        return true;
    }

private:
    static float smoothstep(float t)
    {
        return t * t * (3.0f - 2.0f * t);
    }

    float noise(float time, float freq, float seed)
    {
        float t = time * freq + seed;
        float xi = std::floor(t);
        float xf = t - xi;

        float a = hash(int(xi) & 0xff, int(seed) & 0xff);
        float b = hash((int(xi) + 1) & 0xff, int(seed) & 0xff);

        return a + smoothstep(xf) * (b - a);
    }

    static float hash(int ix, int iy)
    {
        unsigned char h = (ix * 137 + iy * 251) & 0xff;
        return h / 255.0f * 2.0f - 1.0f;
    }

    float _duration = 0.0f;
    float _elapsed = 0.0f;
    float _value = 0.0f;
};
