#pragma once

#include <QByteArray>
#include <QMutex>
#include <QString>
#include <QVector>

#include "ttsconfig.hpp"
#include "voicevoxiface.hpp"

// voicevox 的进程内门面。公共 API 与插件化之前逐字一致，
// 内部转调 VoicevoxHost 提供的 IVoicevoxTts 插件实例。
class VoicevoxTTS
{
public:
    using StyleInfo = IVoicevoxTts::StyleInfo;
    using SpeakerInfo = IVoicevoxTts::SpeakerInfo;

    bool initialize(const QString &dictDir);
    bool loadModel(const QString &modelPath);
    static VoicevoxTTS &instance();

    /** 应用配置并自动处理辞书加载和模型切换（仅当路径变化时重新加载） */
    bool applyConfig(const TTSConfig &config);

    QVector<SpeakerInfo> getSpeakers() const;
    QByteArray synthesis(const QString &text, int styleId, double speed = 1.0);
    bool isReady() const;
    void unloadModel();

    /**
     * @brief 使用当前配置合成语音并保存到文件
     * @param config TTS 配置（用于应用及文件名哈希）
     * @param text   日语文本
     * @param styleId 风格 ID
     * @param speed  语速（speedScale，>0 生效，默认 1.0）
     * @return 成功返回文件路径，失败返回空字符串
     */
    QString synthesizeToFile(const TTSConfig &config, const QString &text, int styleId, double speed);

    /** 返回当前已加载模型的所有风格 ID（纯工具方法） */
    QVector<int> getStyleIds() const;

private:
    VoicevoxTTS() = default;
    ~VoicevoxTTS() = default;
    VoicevoxTTS(const VoicevoxTTS &) = delete;
    VoicevoxTTS &operator=(const VoicevoxTTS &) = delete;

    QRecursiveMutex m_mutex;
};
