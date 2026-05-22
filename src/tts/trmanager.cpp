#include "trmanager.h"
#include "translator.h"
#include "libretranslateclient.h"

TrManager::TrManager(QObject *parent)
    : QObject(parent),
      m_legacyTranslator(Translator::instance()),
      m_libreClient(new LibreTranslateClient(this)),
      m_tencentClient(new TencentTranslator(this))
{
    // 连接信号，方便外部统一监听
    connect(m_libreClient, &LibreTranslateClient::translationFinished,
            this, &TrManager::translationFinished);
    connect(m_libreClient, &LibreTranslateClient::translationError,
            this, &TrManager::translationError);

    connect(m_legacyTranslator, &Translator::translationFinished,
            this, &TrManager::translationFinished);
    connect(m_legacyTranslator, &Translator::translationError,
            this, &TrManager::translationError);
    // TencentTranslator
    connect(m_tencentClient, &TencentTranslator::translationFinished,
            this, &TrManager::translationFinished);
    connect(m_tencentClient, &TencentTranslator::translationError,
            this, &TrManager::translationError);
}

TrManager *TrManager::instance()
{
    static TrManager mgr;
    return &mgr;
}

void TrManager::setConfig(const TTSConfig &config)
{
    m_config = config;
    m_configValid = true;

    // 应用端口配置到 LibreTranslateClient
    if (!m_config.tr_libretranslate_port.isEmpty())
    {
        bool ok;
        int port = m_config.tr_libretranslate_port.toInt(&ok);
        if (ok)
        {
            m_libreClient->setPort(port);
        }
        else
        {
            qWarning() << "[TrManager] Invalid LibreTranslate port:" << m_config.tr_libretranslate_port;
        }
    }
    // 应用腾讯云配置
    if (m_config.tr_point == 2)
    {
        m_tencentClient->setCredentials(
            m_config.tr_tx_secret_id,
            m_config.tr_tx_secret_key,
            m_config.tr_tx_region.isEmpty() ? QStringLiteral("ap-guangzhou") : m_config.tr_tx_region,
            m_config.tr_tx_project_id);
    }
}

void TrManager::translate(const QString &text)
{
    if (!m_configValid)
    {
        emit translationError("Translation config not set");
        return;
    }

    switch (m_config.tr_point)
    {
    case 0:
    {
        QString targetLang = m_config.tr_lang_libretranslate;
        if (targetLang.isEmpty())
        {
            emit translationError("LibreTranslate target language not configured");
            return;
        }
        m_libreClient->translate(text, targetLang);
        break;
    }
    case 1:
    {
        QString targetLang = m_config.tr_lang_translators;
        QString provider = m_config.tr_provider;
        if (targetLang.isEmpty() || provider.isEmpty())
        {
            emit translationError("Remote translator target language or provider not configured");
            return;
        }
        m_legacyTranslator->translate(text, targetLang, provider);
        break;
    }
    case 2:
    {
        QString targetLang = m_config.tr_tx_target_lang;
        if (targetLang.isEmpty())
        {
            emit translationError("Tencent target language not configured");
            return;
        }
        QString sourceLang = m_config.tr_tx_source_lang.isEmpty() ? QStringLiteral("auto") : m_config.tr_tx_source_lang;
        m_tencentClient->translate(text, targetLang, sourceLang);
        break;
    }
    default:
    {
        emit translationError(QString("Unknown translation point: %1").arg(m_config.tr_point));
        break;
    }
    }
}