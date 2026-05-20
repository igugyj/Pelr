
#include "tray.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QTime>
#include <QDebug>
#include <QFile>
#include "launcher.hpp"
#include "data.hpp"
#include "launcherMenu.hpp"
#include "custommenu.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <endpointvolume.h>
#include <iostream>
#include "AudioSpectrumDetector.h"
// 初始化静态成员变量
TrayIcon *TrayIcon::m_instance = nullptr;

TrayIcon *TrayIcon::instance()
{
    if (!m_instance)
    {
        m_instance = new TrayIcon();
    }
    return m_instance;
}

void TrayIcon::showMessage(const QString &title, const QString &msg,
                           QSystemTrayIcon::MessageIcon icon, int timeout)
{
    if (m_instance && !m_instance->m_silentMode)
    {
        m_instance->QSystemTrayIcon::showMessage(title, msg, icon, timeout);
    }
}

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    setIcon(m_appIcon);
    setToolTip(
        DataManager::instance().Project_Name + " " + DataManager::instance().const_config_data.version);

    m_rotateTimer = new QTimer(this);
    connect(m_rotateTimer, &QTimer::timeout, this, &TrayIcon::rotateNote);

    m_audioCheckTimer = new QTimer(this);
    connect(m_audioCheckTimer, &QTimer::timeout, this, &TrayIcon::checkAudioActivity);
    m_audioCheckTimer->setInterval(500);

    // 创建右键菜单
    menu = new CustomMenu();

    action_resetWinLoc = new QAction(tr("重置位置"), this);
    action_showWin = new QAction(tr("显示界面"), this);
    action_silentMode = new QAction(tr("静默模式"), this);
    action_switchDrag = new QAction(tr("锁定位置"), this);
    action_mediaPlayer = new QAction(tr("播放媒体"), this);

    action_quit = new QAction(tr("退出程序"), this);
    action_keyListener = new QAction(tr("按键监听"), this);

    action_silentMode->setCheckable(true);
    action_switchDrag->setCheckable(true);
    action_keyListener->setCheckable(true);

    QMenu *MenuOpen = new CustomMenu(tr("打开"), menu);

    QAction *action_openDirPath = new QAction(tr("程序文件夹"), MenuOpen);
    QAction *action_openUserPath = new QAction(tr("用户文件夹"), MenuOpen);
    QAction *action_openLogPath = new QAction(tr("日志文件夹"), MenuOpen);
    // 连接信号和槽
    // 打开程序文件夹
    connect(action_openDirPath, &QAction::triggered, []()
            {
        const QString appDir = QCoreApplication::applicationDirPath();
        launchByPath(appDir); });
    // 打开用户文件夹
    connect(action_openUserPath, &QAction::triggered, []()
            {
        QString appDir = QCoreApplication::applicationDirPath();
        const QString userDir = appDir.append("/user");
        launchByPath(userDir); });
    // 打开日志文件夹
    connect(action_openLogPath, &QAction::triggered, []()
            {
        QString appDir = QCoreApplication::applicationDirPath();
        const QString logDir = appDir.append("/log");
        launchByPath(logDir); });
    QAction *action_startApp = new QAction("启动项目", this);
    action_startApp->setMenu(launcherMenu::instance());
    // 如果有内容就添加到菜单
    if (launcherMenu::instance()->hasContent)
    {
        // 添加菜单项到菜单
        menu->addAction(action_startApp);
        menu->addSeparator();
    }
    menu->addActions({action_silentMode, action_switchDrag,
                      action_keyListener,
                      action_showWin, action_resetWinLoc, action_mediaPlayer});
    MenuOpen->addActions({action_openUserPath, action_openDirPath, action_openLogPath});
    menu->addSeparator();
    menu->addMenu(MenuOpen);
    menu->addSeparator();
    menu->addAction(action_quit);

    // 设置托盘图标的菜单
    this->setContextMenu(menu);

    qDebug() << "TrayIcon singleton initialized";
    // 显示托盘图标
    this->show();
    switchMusicIcon(DataManager::instance().getBasicData().isMusicIcon);
}

void TrayIcon::initializeAudioDetector()
{
    m_audioDetector = new AudioSpectrumDetector();
    if (m_audioDetector->start())
    {
        m_audioCheckTimer->start();
        qDebug() << "Spectrum-based audio detector initialized";
    }
    else
    {
        qDebug() << "Audio spectrum detector initialization failed";
        delete m_audioDetector;
        m_audioDetector = nullptr;
    }
}

QPixmap TrayIcon::createMusicIcon() const
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 应用旋转
    if (m_rotating)
    {
        painter.translate(32, 32);
        painter.rotate(m_angle);
        painter.translate(-32, -32);
    }

    // 设置字体和颜色
    QFont font(DataManager::instance()._font);
    font.setPointSize(64);
    font.setBold(true);
    painter.setFont(font);

    // 根据旋转状态改变颜色
    if (m_rotating)
    {
        painter.setPen(QColor(DataManager::instance().getBasicData().color_tray.background)); // 有音频
    }
    else
    {
        painter.setPen(QColor(DataManager::instance().getBasicData().color_tray.forground)); // 无音频
    }

    // 绘制音乐符号
    painter.drawText(pixmap.rect(), Qt::AlignCenter, DataManager::instance().getBasicData().music_tray_symbol);

    return pixmap;
}

void TrayIcon::rotateNote()
{
    m_angle = (m_angle + 30) % 360;
    setIcon(QIcon(createMusicIcon()));
}

void TrayIcon::checkAudioActivity()
{
    if (!m_audioDetector)
        return;

    bool isAudioPlaying = m_audioDetector->isAudioPlaying();
    float energy = m_audioDetector->currentEnergy();
    // qDebug() << "Energy:" << energy << "Active:" << isAudioPlaying;
    // 根据音频状态控制旋转
    if (isAudioPlaying && !m_rotating)
    {
        // 开始旋转
        m_rotating = true;
        m_rotateTimer->start(100); // 固定速度：100ms
    }
    else if (!isAudioPlaying && m_rotating)
    {
        // 停止旋转
        m_rotating = false;
        m_rotateTimer->stop();
    }

    // 更新图标以反映颜色变化
    setIcon(QIcon(createMusicIcon()));
}

void TrayIcon::switchMusicIcon(const bool flag)
{
    qDebug() << "switchMusicIcon called with flag:" << flag;
    if (flag)
    {
        initializeAudioDetector();
        qDebug() << "music icon enabled";
    }
    else
    {
        m_audioCheckTimer->stop();
        m_rotateTimer->stop();
        m_rotating = false;
        setIcon(m_appIcon);

        if (m_audioDetector)
        {
            m_audioDetector->stop();
            delete m_audioDetector;
            m_audioDetector = nullptr;
        }
        qDebug() << "music icon disabled";
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
    if (m_audioDetector)
    {
        m_audioDetector->stop();
        delete m_audioDetector;
    }
}
