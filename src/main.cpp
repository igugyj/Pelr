#include <QApplication>
#include <QCoreApplication>
#include <QStyleFactory>
#include <QIcon>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "GLCore.h"
#include "tray.h"
#include "CheckApplication.h"
#include "TranslationManager.h"
#include "logger.hpp"
#include "NotificationWidget.h"
#include "initFileSys.h"
#include "voicevox_tts.h"

void initTranslator(QApplication &a, const QString &path);

int main(int argc, char *argv[])
{
    // ---- 基础初始化（应在 QApplication 之前完成，但注意不要依赖 QSettings 等） ----
    initFileSys();
    initLogFile();

#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif

    setLogLevel(read_log_level());

#ifndef CONSOLE // Release 模式（无控制台）才安装消息处理器
    qInstallMessageHandler(messageHandler);
#endif

    QApplication app(argc, argv);

    // 尽早初始化翻译，让后续对话框可使用 tr()
    initTranslator(app, ":/translations");

    app.setApplicationName(DataManager::instance().Project_Name + " " + DataManager::instance().const_config_data.version);
    // 确保 DataManager 已加载字体
    QFont main_font = DataManager::instance()._font;
    main_font.setWeight(QFont::Medium);
    app.setFont(main_font);
    app.setWindowIcon(QIcon(":/public/image/Pelr.png"));

    // 样式设置
    QStringList styles = QStyleFactory::keys();
    for (auto &style : styles)
    {
        qDebug() << "[APP] Available style:" << style;
    }
    if (styles.contains("FluentUI3"))
        app.setStyle("FluentUI3");
    else if (styles.contains("Fusion", Qt::CaseInsensitive))
        app.setStyle("Fusion");

    // 许可证检查
    if (!CheckApplication::hasValidLicense())
    {
        CheckApplication licenseDialog;
        if (licenseDialog.exec() != QDialog::Accepted || !licenseDialog.isLicenseAccepted())
        {
            qDebug() << "[APP] License not accepted, exit";
            return 1; // 非零退出码表示异常
        }
        qDebug() << "[APP] License accepted";
    }

    QObject::connect(&app, &QCoreApplication::aboutToQuit, TrayIcon::cleanup);

    TrayIcon::instance()->show();

    // ONNX 初始化（如可能耗时，可考虑异步，此处保持简单）
    if (!VoicevoxTTS::initializeOnnxRuntime())
    {
        qWarning() << "[APP] Failed to initialize OnnxRuntime";
    }

    // 根据静默启动选项决定是否显示主窗口
    GLCore w;
    if (DataManager::instance().getBasicData().isSilentBoot)
    {
        TrayIcon::instance()->action_silentMode->triggered();
    }
    else
    {
        w.show();
    }

    return app.exec();
}

void initTranslator(QApplication &a, const QString &path)
{
    TranslationManager::setApplication(&a);
    TranslationManager::instance()->addTranslationPath(path);

    // 1. 优先读取配置
    QString lang = DataManager::instance().getBasicData().language;

    // 2. 配置为空 → 检测系统语言
    if (lang.isEmpty())
        lang = TranslationManager::instance()->detectSystemLanguage();

    // 3. 不支持 → 默认英语
    TranslationManager::instance()->setLanguage(lang);
    qDebug() << "[APP] Language set to:" << lang;
    qDebug() << "[APP] Translator initialized";
}