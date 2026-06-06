
#include "TranslationManager.h"
#include <QDebug>
#include <QLibraryInfo>
#include <QSettings>
#include <QStandardPaths>

// 初始化静态成员
TranslationManager *TranslationManager::m_instance = nullptr;
QApplication *TranslationManager::m_app = nullptr;

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent), m_translator(new QTranslator(this)), m_currentLanguage("en_US")
{
    // 初始化默认路径
    m_translationPaths << ":/translations";
    m_translationPaths << QApplication::applicationDirPath() + "/translations";
    m_translationPaths << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/translations";

    // 初始化语言列表
    initLanguages();
}

void TranslationManager::setApplication(QApplication *app)
{
    m_app = app;
}

TranslationManager *TranslationManager::instance()
{
    if (!m_instance)
    {
        m_instance = new TranslationManager();
    }
    return m_instance;
}

void TranslationManager::initLanguages()
{
    m_languages["en_US"] = tr("English");
    m_languages["zh_CN"] = tr("简体中文");
}

void TranslationManager::addTranslationPath(const QString &path)
{
    if (!m_translationPaths.contains(path))
    {
        m_translationPaths.prepend(path); // 添加到最前面优先搜索
    }
}

QString TranslationManager::findTranslationFile(const QString &languageCode)
{
    QStringList possibleNames;
    possibleNames << QString("%1.qm").arg(languageCode);
    possibleNames << QString("language_%1.qm").arg(languageCode);
    possibleNames << QString("languages_%1.qm").arg(languageCode);
    possibleNames << QString("qt_%1.qm").arg(languageCode);
    possibleNames << QString("%1.ts").arg(languageCode);
    possibleNames << QString("language_%1.ts").arg(languageCode);
    possibleNames << QString("languages_%1.ts").arg(languageCode);

    // 搜索所有路径
    for (const QString &path : m_translationPaths)
    {
        QDir dir(path);
        if (dir.exists())
        {
            for (const QString &name : possibleNames)
            {
                QString filePath = dir.absoluteFilePath(name);
                if (QFile::exists(filePath))
                {
                    return filePath;
                }
            }
        }
    }

    return QString();
}

bool TranslationManager::loadTranslation(const QString &languageCode)
{
    if (languageCode.isEmpty())
    {
        qWarning() << "[L10n] Language code is empty";
        return false;
    }

    // 移除旧的翻译器
    if (m_app)
    {
        m_app->removeTranslator(m_translator);
        delete m_translator;
        m_translator = new QTranslator(this);
    }

    // 查找翻译文件
    QString translationFile = findTranslationFile(languageCode);

    if (translationFile.isEmpty())
    {
        qWarning() << "[L10n] Translation file not found for language:" << languageCode;
        return false;
    }

    // 加载翻译文件
    if (m_translator->load(translationFile))
    {
        if (m_app)
        {
            m_app->installTranslator(m_translator);
        }

        // 加载Qt标准库的翻译（可选）
        QTranslator *qtTranslator = new QTranslator(this);
        if (qtTranslator->load(QString("qt_%1").arg(languageCode),
                               QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        {
            if (m_app)
            {
                m_app->installTranslator(qtTranslator);
            }
        }

        m_currentLanguage = languageCode;
        qDebug() << "[L10n] Language changed to:" << languageCode;

        // 发射信号
        emit languageChanged(languageCode);

        return true;
    }

    qWarning() << "[L10n] Failed to load translation file:" << translationFile;
    return false;
}

void TranslationManager::setLanguage(const QString &languageCode)
{
    if (m_currentLanguage == languageCode)
    {
        return;
    }

    if (loadTranslation(languageCode))
    {
        m_currentLanguage = languageCode;
    }
}

QString TranslationManager::currentLanguage() const
{
    return m_currentLanguage;
}

QStringList TranslationManager::availableLanguages() const
{
    return m_languages.keys();
}

QString TranslationManager::detectSystemLanguage()
{
    // 获取系统语言环境
    QString locale = QLocale::system().name();

    // 检查是否支持该系统语言
    if (m_languages.contains(locale))
    {
        return locale;
    }

    // 尝试使用基础语言代码（例如zh_CN -> zh）
    QString baseLang = locale.split('_').first();
    for (const QString &lang : m_languages.keys())
    {
        if (lang.startsWith(baseLang))
        {
            return lang;
        }
    }

    // 默认返回英语
    return "en_US";
}
