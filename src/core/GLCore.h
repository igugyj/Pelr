
#pragma once

#include <QMenu>
#include <QWidget>
#include <QOpenGLWidget>
#include <QTimer>
#include <QPoint>
#include "globalinputlistener.h"
#include <QLabel>
#include <QWheelEvent>
#include <QVariantAnimation>
#include "MenuWidget.h"
#include "keyboardoverlay.h"
#include <QPair>
#include "mainWidget.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include "chatBoxOnModel.h"
#include "ContextMenu.h"
// #include "recorder.hpp"
// https://www.bilibili.com/video/BV1kcc8emEfR

class GLCore : public QOpenGLWidget
{
Q_OBJECT // 可以用信号槽机制进行通信 信号与槽
    public : QTimer *timer;
    QTimer *inputCheckTimer;
    ContextMenu *contextMenu;
    QTimer *PermanentTimer;
    KeyboardOverlay *overlay;
    QTimer *randomSentenceTimer;
    mainWidget *main_widget;
    ChatBoxOnModel *modelChatBox;
    GlobalInputListener *listener;

    void handleClick(const QPoint &localPos);

    void switchWindowTransparent(bool transparent);

    void switchListener();

    GLCore(QWidget *parent = nullptr);

    ~GLCore();

public slots:
    void resetLocation();

    void loadModel();

    void silentMode();

    void switchDragStatus();

private slots:
    // 鼠标透明度检查->是否进行鼠标穿透
    void checkMouseTransparency();

    void checkFocus();

    void saveWindowLocation();

    void loadWindowLocation();

private:
    bool m_glInitialized = false;
    // buttons actions
    QPushButton *switchListenerButton;
    QPushButton *RandomSentenceButton;
    QPushButton *QuickStartButton;
    QAction *askWeather;
    QAction *askPowerStatus;
    QAction *askLatestNextTodoEvent;
    QPushButton *QuestionButton;
    QPushButton *SettingButton;
    QPushButton *EmotionButton;
    QPushButton *MediaButton;
    // bool flags
    bool isFocused = false;
    bool isAllowDrag = true;
    bool isSilentMode = false;
    bool left_button_down = false;
    bool right_button_down = false;
    bool m_currentlyTransparent = false;
    QPoint dragStartPos;
    // QPair<long, long> keyCounter = QPair<long, long>(0, 0);
    // QTimer *keyCounterTimer;
    // Recorder *recorder;
    QFutureWatcher<void> m_watcher;

    void connectSignals();

    void startRunStarIfPoweredInThread();

    void runStarIfPowered();

    void onRunStarIfPoweredFinished();

    void openSetting();

protected:
    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    void showEvent(QShowEvent *event) override;
};
