
#pragma once

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QList>
#include <QPixmap>
#include "data.hpp"

class AudioSpectrumDetector;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    TrayIcon(const TrayIcon &) = delete;
    TrayIcon &operator=(const TrayIcon &) = delete;

    static TrayIcon *instance();
    static void cleanup();

    static void showMessage(const QString &title, const QString &msg,
                            QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                            int timeout = 10000);

    void setTrayIconMode(int mode, const QString &gifPath = QString());

    QAction *action_resetWinLoc;
    QAction *action_showWin;
    QAction *action_silentMode;
    QAction *action_switchDrag;
    QAction *action_mediaPlayer;
    QAction *action_quit;
    QAction *action_keyListener;

    void retranslateUI();
    void switchLaunchMenu(const bool flag);

private:
    TrayIcon(QObject *parent = nullptr);
    ~TrayIcon();

    static TrayIcon *m_instance;

    QMenu *menu;
    QMenu *m_menuOpen;
    QAction *m_actionOpenDirPath;
    QAction *m_actionOpenUserPath;
    QAction *m_actionOpenLogPath;
    QAction *m_actionStartApp;
    bool m_silentMode = false;

    void initializeAudioDetector();
    void stopAudioDetector();
    QTimer *m_audioCheckTimer;
    AudioSpectrumDetector *m_audioDetector = nullptr;

    int m_mode = TrayIcon_Static;
    QIcon m_appIcon;

    // Text mode state
    QTimer *m_textTimer;
    int m_textAngle = 0;
    bool m_textRotating = false;

    // GIF mode state
    struct GifFrame
    {
        QPixmap pixmap;
        int delayMs;
    };
    QList<GifFrame> m_gifFrames;
    int m_gifIdx = 0;
    QTimer *m_gifTimer;

private slots:
    void checkAudioActivity();
    void textRotate();
    void loadGifFrames(const QString &path);
    void advanceGifFrame();
};
