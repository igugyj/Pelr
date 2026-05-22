#ifndef TRMANAGER_H
#define TRMANAGER_H

#include <QObject>
#include <QString>
#include "data.hpp"
#include "tencenttranslator.h"
// 前置声明
class Translator;
class LibreTranslateClient;

/**
 * @brief 聚合翻译后端，根据配置自动选择
 */
class TrManager : public QObject
{
    Q_OBJECT

public:
    static TrManager *instance();

    // 设置配置（必须在使用 translate 前调用）
    void setConfig(const TTSConfig &config);

    // 根据当前配置执行翻译
    void translate(const QString &text);

signals:
    void translationFinished(const QString &translatedText);
    void translationError(const QString &errorMessage);

private:
    explicit TrManager(QObject *parent = nullptr);
    ~TrManager() override = default;

    TrManager(const TrManager &) = delete;
    TrManager &operator=(const TrManager &) = delete;

    Translator *m_legacyTranslator;
    LibreTranslateClient *m_libreClient;
    TencentTranslator *m_tencentClient;

    TTSConfig m_config;
    bool m_configValid = false;
};

#endif // TRANSLATIONMANAGER_H