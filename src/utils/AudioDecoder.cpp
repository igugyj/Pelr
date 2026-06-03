#include "AudioDecoder.hpp"
#include <miniaudio.h>
#include <QDebug>
#include <QFile>

DecodedAudio AudioDecoder::decode(const QString &filePath)
{
    DecodedAudio result;

    if (!QFile::exists(filePath))
    {
        qWarning() << "[AudioDecoder] File not found:" << filePath;
        return result;
    }

    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);

    if (ma_decoder_init_file(filePath.toUtf8().constData(), &config, &decoder) != MA_SUCCESS)
    {
        qWarning() << "[AudioDecoder] Failed to init decoder for:" << filePath;
        return result;
    }

    ma_uint64 totalFrames;
    if (ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames) != MA_SUCCESS)
    {
        qWarning() << "[AudioDecoder] Failed to get PCM frame count";
        ma_decoder_uninit(&decoder);
        return result;
    }

    result.sampleRate = decoder.outputSampleRate;
    result.channels = decoder.outputChannels;
    result.totalFrameCount = totalFrames;

    result.pcmData.resize(static_cast<size_t>(totalFrames) * result.channels);

    ma_uint64 framesRead;
    ma_result readResult = ma_decoder_read_pcm_frames(&decoder, result.pcmData.data(), totalFrames, &framesRead);
    if (readResult != MA_SUCCESS && readResult != MA_AT_END)
    {
        qWarning() << "[AudioDecoder] Read error:" << readResult;
        ma_decoder_uninit(&decoder);
        return result;
    }

    result.pcmData.resize(static_cast<size_t>(framesRead) * result.channels);
    result.totalFrameCount = framesRead;
    result.valid = true;

    ma_decoder_uninit(&decoder);
    return result;
}
