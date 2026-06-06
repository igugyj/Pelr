
#ifndef TRAY_H
#define TRAY_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class AudioSpectrumDetector;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    // 删除拷贝构造函数和赋值运算符
    TrayIcon(const TrayIcon &) = delete;

    TrayIcon &operator=(const TrayIcon &) = delete;

    // 获取单例实例
    static TrayIcon *instance();

    // 程序退出时清理单例
    static void cleanup();

    // 静态方法用于显示消息
    static void showMessage(const QString &title, const QString &msg,
                            QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                            int timeout = 10000);

    QAction *action_resetWinLoc;
    QAction *action_showWin;
    QAction *action_silentMode;
    QAction *action_switchDrag;
    QAction *action_mediaPlayer;
    QAction *action_quit;
    QAction *action_keyListener;

    void retranslateUI();

    QIcon m_appIcon = QIcon(":/public/image/Pelr.png");

private:
    TrayIcon(QObject *parent = nullptr);

    ~TrayIcon();

    // 单例实例指针
    static TrayIcon *m_instance;

    QMenu *menu;
    QMenu *m_menuOpen;
    QAction *m_actionOpenDirPath;
    QAction *m_actionOpenUserPath;
    QAction *m_actionOpenLogPath;
    QAction *m_actionStartApp;
    bool m_silentMode = false;

    QPixmap createMusicIcon() const;

    void initializeAudioDetector();

    bool m_rotating = false;
    int m_angle = 0;
    QTimer *m_rotateTimer;
    QTimer *m_audioCheckTimer;
    // Audio detection members
    AudioSpectrumDetector *m_audioDetector = nullptr;

public:
    void switchMusicIcon(bool flag);

private slots:
    void rotateNote();

    void checkAudioActivity();
};

#endif // TRAY_H
