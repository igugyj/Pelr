
#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDebug>
#include "data.hpp"
#include "launcher.hpp"
#include "translator.h"
#include "trmanager.h"
#include "voicevox_tts.h"

class VoiceGenerator : public QObject
{
    Q_OBJECT

public:
    static VoiceGenerator *instance()
    {
        static VoiceGenerator instance;
        return &instance;
    }

    /**
     * @brief 主语音生成入口，根据配置自动处理翻译（如果需要）然后TTS
     * @param config TTS及翻译配置（不再需要包含 text）
     * @param text   待合成（或待翻译）的原始文本
     */
    void generateVoice(const TTSConfig &config, const QString &text)
    {

        m_pendingConfig = config;
        m_pendingText = text;
        TrManager::instance()->setConfig(config);
        TrManager::instance()->translate(text);
    }

    // 原有讯飞 TTS 调用方式（保持不变）
    void generateVoiceIFlytek(const QString &appid, const QString &apiKey, const QString &apiSecret,
                              const QString &text, const QString &voice = "x4_yezi")
    {
        QString speaker = voice;
        if (speaker.isEmpty())
        {
            speaker = "x4_yezi";
        }
        QJsonObject json;
        json["appid"] = appid;
        json["api_key"] = apiKey;
        json["api_secret"] = apiSecret;
        json["text"] = text;
        json["voice"] = speaker;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        QNetworkRequest request(QUrl("http://localhost:9140/generate"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = m_manager->post(request, data);
        connect(reply, &QNetworkReply::finished, this, [this, reply]()
                { handleGenerateResponse(reply); });
    }

    // 通过远端 OpenAI 兼容 API 合成语音
    void generateVoiceOpenAICompat(const QString &text,
                                   const QString &endpoint,
                                   const QString &apiKey,
                                   const QString &model,
                                   const QString &voice,
                                   double speed)
    {
        qDebug() << "[VoiceGen] OpenAI-Compatible: text=" << text.left(50)
                 << "endpoint=" << endpoint << "model=" << model << "voice=" << voice << "speed=" << speed;

        if (endpoint.isEmpty())
        {
            qWarning() << "[VoiceGen] OpenAI-Compatible endpoint is empty, aborting";
            emit errorOccurred("OpenAI TTS endpoint not configured");
            return;
        }

        QString hashInput = text + "|" + voice + "|" + model + "|" + QString::number(speed);
        QByteArray hash = QCryptographicHash::hash(hashInput.toUtf8(), QCryptographicHash::Sha256).toHex();

        QString voiceDir = DataManager::instance().const_config_data.VoiceFolder;
        QDir().mkpath(voiceDir);
        QString filePath = voiceDir + "/" + hash + ".wav";

        if (QFile::exists(filePath))
        {
            qDebug() << "[VoiceGen] Reusing cached:" << filePath;
            emit voiceGenerated(filePath);
            return;
        }

        double finalSpeed = qBound(0.25, speed, 4.0);

        QJsonObject json;
        json["model"] = model;
        json["input"] = text;
        json["voice"] = voice;
        json["response_format"] = "wav";
        json["speed"] = finalSpeed;

        QByteArray data = QJsonDocument(json).toJson();

        QUrl url(endpoint.trimmed());
        if (!url.isValid())
        {
            qWarning() << "[VoiceGen] Invalid URL:" << url.toString();
            emit errorOccurred("Invalid OpenAI TTS URL: " + url.errorString());
            return;
        }

        qDebug() << "[VoiceGen] POST" << url.toString();
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        if (!apiKey.isEmpty())
            request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

        QNetworkReply *reply = m_manager->post(request, data);
        connect(reply, &QNetworkReply::finished, this, [this, reply, filePath]()
                { handleOpenAICompatResponse(reply, filePath); });
    }

    // 新增：OpenAI 风格 TTS 调用（Edge TTS）
    void generateVoiceOpenAI(const QString &text, const QString &voice = "zh-CN-XiaoxiaoNeural",
                             double speed = 1.0)
    {
        double finalSpeed = qBound(0.25, speed, 4.0);

        QJsonObject json;
        json["input"] = text;
        json["voice"] = voice;
        json["response_format"] = "mp3";
        json["speed"] = finalSpeed;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        QNetworkRequest request(QUrl("http://localhost:9140/v1/audio/speech_local"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = m_manager->post(request, data);
        connect(reply, &QNetworkReply::finished, this, [this, reply]()
                { handleOpenAIResponse(reply); });
    }

    void playVoice(const QString &filePath)
    {
        if (!m_player)
            return;

        if (m_currentPlayFile == filePath)
        {
            if (m_player->playbackState() == QMediaPlayer::PlayingState)
            {
                m_player->stop();
            }
            m_player->setSource(QUrl::fromLocalFile(filePath));
            m_player->play();
            return;
        }

        if (!m_currentPlayFile.isEmpty())
        {
            m_player->stop();
            m_currentPlayFile.clear();
        }

        m_currentPlayFile = filePath;
        if (!QFile::exists(filePath))
        {
            qWarning() << "[VoiceGen] File does not exist:" << filePath;
            emit errorOccurred(tr("Audio file not found: %1").arg(filePath));
            return;
        }

        m_audioOutput->setVolume(DataManager::instance().getBasicData().volume / 100.0);
        m_player->setSource(QUrl::fromLocalFile(filePath));
        m_player->play();
    }

    void stopVoice()
    {
        if (m_player)
        {
            m_player->stop();
        }
    }

signals:
    void voiceGenerated(const QString &filePath);
    void errorOccurred(const QString &errorMessage);
    void playbackFinished();

private slots:
    // 翻译成功后的处理
    void onTranslationFinished(const QString &translatedText)
    {
        qDebug() << "[VoiceGen] Translation successful:" << translatedText;
        doGenerateVoice(m_pendingConfig, translatedText);
    }

    void onTranslationError(const QString &errorMessage)
    {
        qWarning() << "[VoiceGen] Translation failed:" << errorMessage;
        emit errorOccurred("Translation failed: " + errorMessage);
        doGenerateVoice(m_pendingConfig, m_pendingText); // 回退原文合成
    }

private:
    VoiceGenerator(QObject *parent = nullptr) : QObject(parent)
    {
        m_manager = new QNetworkAccessManager(this);

        m_player = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput();
        m_player->setAudioOutput(m_audioOutput);
        m_audioOutput->setVolume(0.5);

        connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state)
                {
            if (state == QMediaPlayer::StoppedState) {
                emit playbackFinished();
            } });

        // 连接翻译信号
        TrManager *transMgr = TrManager::instance();
        connect(transMgr, &TrManager::translationFinished,
                this, &VoiceGenerator::onTranslationFinished);
        connect(transMgr, &TrManager::translationError,
                this, &VoiceGenerator::onTranslationError);
    }

    VoiceGenerator(const VoiceGenerator &) = delete;
    VoiceGenerator &operator=(const VoiceGenerator &) = delete;

    // 统一执行 TTS 生成
    void doGenerateVoice(const TTSConfig &config, const QString &text)
    {
        qDebug() << "[VoiceGen] doGenerateVoice provider=" << config.provider << "text=" << text.left(50);
        switch (config.provider)
        {
        case 0:
            qDebug() << "[VoiceGen] Using Edge TTS (local Python server)";
            generateVoiceOpenAI(text, config.speaker_openai_edge_tts, config.speed_openai_edge_tts);
            break;
        case 1:
            qDebug() << "[VoiceGen] Using iFlytek TTS";
            generateVoiceIFlytek(config.iFlytek_APPID, config.iFlytek_APIKey, config.iFlytek_APISecret,
                                 text, config.iFlytek_speaker);
            break;
        case 2:
            qDebug() << "[VoiceGen] Using VOICEVOX";
        {
            int styleId = config.voicevox_style_id;
            double speed = config.voicevox_speed;
            (void)QtConcurrent::run([this, config, text, styleId, speed]()
                              {
            QString filePath = VoicevoxTTS::instance().synthesizeToFile(config, text, styleId, speed);
            if (filePath.isEmpty()) {
                emit errorOccurred("VOICEVOX synthesis or save failed");
            } else {
                QMetaObject::invokeMethod(this, [this, filePath]() {
                    emit voiceGenerated(filePath);
                }, Qt::QueuedConnection);
            } });
            break;
        }
        case 3:
            qDebug() << "[VoiceGen] Using OpenAI-Compatible TTS";
            generateVoiceOpenAICompat(text,
                config.openai_endpoint,
                config.openai_apiKey,
                config.openai_model,
                config.openai_voice,
                config.openai_speed);
            break;
        default:
            qWarning() << "[VoiceGen] Unknown provider:" << config.provider;
            emit errorOccurred("Unknown TTS provider");
            break;
        }
    }

    void handleGenerateResponse(QNetworkReply *reply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject json = doc.object();
            qDebug() << "[VoiceGen] Voice generate response:" << json;
            QString filePath = json["file_path"].toString();
            if (!filePath.isEmpty() && QFile::exists(filePath))
            {
                emit voiceGenerated(filePath);
            }
            else
            {
                QString error = json["error"].toString();
                emit errorOccurred(error.isEmpty() ? "File not found" : error);
            }
        }
        else
        {
            if (reply->error() == QNetworkReply::ConnectionRefusedError)
            {
                onConnectionRefused();
            }
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater();
    }

    void handleOpenAIResponse(QNetworkReply *reply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject json = doc.object();
            qDebug() << "[VoiceGen] OpenAI TTS response:" << json;
            QString filePath = json["file_path"].toString();
            if (!filePath.isEmpty() && QFile::exists(filePath))
            {
                emit voiceGenerated(filePath);
            }
            else
            {
                QString error = json["error"].toString();
                emit errorOccurred(error.isEmpty() ? "File not found from OpenAI TTS" : error);
            }
        }
        else
        {
            if (reply->error() == QNetworkReply::ConnectionRefusedError)
            {
                onConnectionRefused();
            }
            emit errorOccurred(QString("OpenAI TTS error: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
    }

    void handleOpenAICompatResponse(QNetworkReply *reply, const QString &filePath)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray audioData = reply->readAll();
            if (audioData.isEmpty())
            {
                emit errorOccurred("OpenAI TTS returned empty audio data");
            }
            else
            {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(audioData);
                    file.close();
                    emit voiceGenerated(filePath);
                }
                else
                {
                    emit errorOccurred("Failed to save audio file: " + filePath);
                }
            }
        }
        else
        {
            emit errorOccurred(QString("OpenAI TTS error: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
    }

    void onConnectionRefused()
    {
        qDebug() << "[VoiceGen] TTS server connection refused";
    }

    QNetworkAccessManager *m_manager;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    TTSConfig m_pendingConfig; // 暂存等待翻译完成的配置
    QString m_pendingText;     // 暂存待翻译的原始文本
    QString m_currentPlayFile; // 当前正在播放（或刚播完）的文件路径
};