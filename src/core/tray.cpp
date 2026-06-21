
#include "tray.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include "launcher.hpp"
#include "launcherMenu.hpp"
#include "custommenu.h"
#include <windows.h>
#include "AudioSpectrumDetector.h"
#include "TranslationManager.h"

TrayIcon *TrayIcon::m_instance = nullptr;

TrayIcon *TrayIcon::instance()
{
    if (!m_instance)
        m_instance = new TrayIcon();
    return m_instance;
}

void TrayIcon::showMessage(const QString &title, const QString &msg,
                           QSystemTrayIcon::MessageIcon icon, int timeout)
{
    if (m_instance && !m_instance->m_silentMode)
        m_instance->QSystemTrayIcon::showMessage(title, msg, icon, timeout);
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent), m_appIcon(":/public/image/Pelr.png")
{
    setIcon(m_appIcon);
    setToolTip(DataManager::instance().Project_Name + " " + DataManager::instance().const_config_data.version);

    m_textTimer = new QTimer(this);
    connect(m_textTimer, &QTimer::timeout, this, &TrayIcon::textRotate);

    m_gifTimer = new QTimer(this);
    connect(m_gifTimer, &QTimer::timeout, this, &TrayIcon::advanceGifFrame);

    m_audioCheckTimer = new QTimer(this);
    connect(m_audioCheckTimer, &QTimer::timeout, this, &TrayIcon::checkAudioActivity);
    m_audioCheckTimer->setInterval(500);

    menu = new CustomMenu();

    action_resetWinLoc = new QAction(tr("Reset Position"), this);
    action_showWin = new QAction(tr("Show Window"), this);
    action_silentMode = new QAction(tr("Silent Mode"), this);
    action_switchDrag = new QAction(tr("Lock Position"), this);
    action_mediaPlayer = new QAction(tr("Play Media"), this);
    action_quit = new QAction(tr("Exit"), this);
    action_keyListener = new QAction(tr("Key Listener"), this);

    action_silentMode->setCheckable(true);
    action_switchDrag->setCheckable(true);
    action_keyListener->setCheckable(true);

    m_menuOpen = new CustomMenu(tr("Open"), menu);

    m_actionOpenDirPath = new QAction(tr("Program Folder"), m_menuOpen);
    m_actionOpenUserPath = new QAction(tr("User Folder"), m_menuOpen);
    m_actionOpenLogPath = new QAction(tr("Log Folder"), m_menuOpen);

    connect(m_actionOpenDirPath, &QAction::triggered, []()
            { launchByPath(QCoreApplication::applicationDirPath()); });
    connect(m_actionOpenUserPath, &QAction::triggered, []()
            { launchByPath(QCoreApplication::applicationDirPath() + "/user"); });
    connect(m_actionOpenLogPath, &QAction::triggered, []()
            { launchByPath(QCoreApplication::applicationDirPath() + "/log"); });

    m_actionStartApp = new QAction(tr("Launch Item"), this);
    m_actionStartApp->setMenu(launcherMenu::instance());
    if (launcherMenu::instance()->hasContent)
    {
        menu->addAction(m_actionStartApp);
        menu->addSeparator();
    }
    menu->addActions({action_silentMode, action_switchDrag,
                      action_keyListener,
                      action_showWin, action_resetWinLoc, action_mediaPlayer});
    m_menuOpen->addActions({m_actionOpenUserPath, m_actionOpenDirPath, m_actionOpenLogPath});
    menu->addSeparator();
    menu->addMenu(m_menuOpen);
    menu->addSeparator();
    menu->addAction(action_quit);

    QFont font = qApp->font();
    font.setWeight(QFont::Medium);
    menu->setFont(font);
    setContextMenu(menu);

    qDebug() << "[Tray] TrayIcon singleton initialized";

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &) { retranslateUI(); });
    setTrayIconMode(DataManager::instance().getBasicData().trayIconMode,
                    DataManager::instance().getBasicData().trayGifPath);
    switchLaunchMenu(DataManager::instance().getBasicData().ShowLaunchMenuinTrayMenu);
    show();
}

void TrayIcon::retranslateUI()
{
    action_resetWinLoc->setText(tr("Reset Position"));
    action_showWin->setText(tr("Show Window"));
    action_silentMode->setText(tr("Silent Mode"));
    action_switchDrag->setText(tr("Lock Position"));
    action_mediaPlayer->setText(tr("Play Media"));
    action_quit->setText(tr("Exit"));
    action_keyListener->setText(tr("Key Listener"));
    m_menuOpen->setTitle(tr("Open"));
    m_actionOpenDirPath->setText(tr("Program Folder"));
    m_actionOpenUserPath->setText(tr("User Folder"));
    m_actionOpenLogPath->setText(tr("Log Folder"));
    m_actionStartApp->setText(tr("Launch Item"));
}
void TrayIcon::switchLaunchMenu(const bool flag)
{
    m_actionStartApp->setVisible(flag);
}

void TrayIcon::setTrayIconMode(int mode, const QString &gifPath)
{
    // Stop previous mode
    m_textTimer->stop();
    m_gifTimer->stop();
    stopAudioDetector();

    m_textRotating = false;
    m_textAngle = 0;
    m_gifFrames.clear();
    m_gifIdx = 0;

    m_mode = mode;

    switch (m_mode)
    {
    case TrayIcon_Static:
        setIcon(m_appIcon);
        qDebug() << "[Tray] Mode set to Static";
        break;

    case TrayIcon_Text:
        textRotate(); // draw static text icon
        initializeAudioDetector();
        qDebug() << "[Tray] Mode set to Text";
        break;

    case TrayIcon_Gif:
        if (!gifPath.isEmpty())
        {
            loadGifFrames(gifPath);
            if (!m_gifFrames.isEmpty())
                setIcon(QIcon(m_gifFrames[0].pixmap));
            else
                setIcon(m_appIcon);
        }
        else
        {
            setIcon(m_appIcon);
        }
        initializeAudioDetector();
        qDebug() << "[Tray] Mode set to GIF";
        break;

    default:
        setIcon(m_appIcon);
        break;
    }
}

void TrayIcon::initializeAudioDetector()
{
    m_audioDetector = new AudioSpectrumDetector();
    if (m_audioDetector->start())
    {
        m_audioCheckTimer->start();
        qDebug() << "[Tray] Spectrum-based audio detector initialized";
    }
    else
    {
        qDebug() << "[Tray] Audio spectrum detector initialization failed";
        delete m_audioDetector;
        m_audioDetector = nullptr;
    }
}

void TrayIcon::stopAudioDetector()
{
    m_audioCheckTimer->stop();
    if (m_audioDetector)
    {
        m_audioDetector->stop();
        delete m_audioDetector;
        m_audioDetector = nullptr;
    }
}

void TrayIcon::checkAudioActivity()
{
    if (!m_audioDetector)
        return;

    bool playing = m_audioDetector->isAudioPlaying();

    switch (m_mode)
    {
    case TrayIcon_Text:
    {
        if (playing && !m_textRotating)
        {
            m_textRotating = true;
            m_textTimer->start(100);
        }
        else if (!playing && m_textRotating)
        {
            m_textRotating = false;
            m_textTimer->stop();
        }
        textRotate(); // update color
        break;
    }
    case TrayIcon_Gif:
    {
        if (playing && !m_gifTimer->isActive())
        {
            if (!m_gifFrames.isEmpty())
            {
                m_gifIdx = 0;
                m_gifTimer->start(m_gifFrames[0].delayMs);
                setIcon(QIcon(m_gifFrames[0].pixmap));
            }
        }
        else if (!playing && m_gifTimer->isActive())
        {
            m_gifTimer->stop();
            m_gifIdx = 0;
            if (!m_gifFrames.isEmpty())
                setIcon(QIcon(m_gifFrames[0].pixmap));
        }
        break;
    }
    default:
        break;
    }
}

void TrayIcon::cleanup()
{
    if (m_instance)
    {
        delete m_instance;
        m_instance = nullptr;
    }
}

TrayIcon::~TrayIcon()
{
    delete menu;
    m_instance = nullptr;
    stopAudioDetector();
}
