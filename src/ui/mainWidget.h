
#pragma once

#include <QWidget>
#include "chat.h"
#include "manageStart.h"
#include "setting.h"
#include "todoWidget.h"
#include "systemMonitorWidget.hpp"

namespace Ui
{
    class mainWidget;
}

class mainWidget : public QWidget
{
    Q_OBJECT

    void setSwitchStyleProperty(const QString &style);

public:
    ChatWidget *Widget_chat;
    ManageStartWidget *Widget_ManageStart;
    SettingWidget *Widget_Setting;
    todoWidget *Widget_Todo;
    SystemMonitorWidget *Widget_SystemMonitor;
    int point_x = 130;
    int point_y = 30;

    explicit mainWidget(QWidget *parent = nullptr);

    ~mainWidget();

    void closeEvent(QCloseEvent *event) override;

    void initUI();

    void retranslateUI();

    void showEvent(QShowEvent *event) override;

private:
    Ui::mainWidget *ui;
};
