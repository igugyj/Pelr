#pragma once
#include <QWidget>
#include <QPushButton>
#include <QMenu>
#include "MenuWidget.h"

class ContextMenu : public MenuWidget
{
    Q_OBJECT
public:
    ContextMenu(QWidget *parent);
    void init();

    QPushButton *m_switchListenerButton;
    QPushButton *m_RandomSentenceButton;
    QPushButton *m_QuickStartButton;
    QPushButton *m_SettingButton;
    QPushButton *m_EmotionButton;
    QPushButton *m_MediaButton;
    QPushButton *m_QuestionButton;
    QMenu *m_QuestionMenu;
    QAction *m_askWeather;
    QAction *m_askPowerStatus;
    QAction *m_askLatestNextTodoEvent;

private:
    void setDefaultMsg();
private slots:
    void onAskWeather();
    void onPlayMedia();
    void retranslateUI();
};