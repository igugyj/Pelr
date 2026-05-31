
#include "mainWidget.h"
#include "ui_mainWidget.h"
#include <QCloseEvent>
#include <QFile>
#include <QIcon>
#include <QDebug>
#include <QMessageBox>

mainWidget::mainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::mainWidget)
{
    ui->setupUi(this);
    initUI();
}

void mainWidget::initUI()
{
    this->setWindowIcon(qApp->windowIcon());
    this->setWindowTitle(qApp->applicationName());
    // QFile styleFile(":/thirdParty/QSS/Ubuntu.qss");
    // if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     QString styleSheet = QLatin1String(styleFile.readAll());
    //     this->setStyleSheet(styleSheet);
    //     styleFile.close();
    // }
    Widget_SystemMonitor = new SystemMonitorWidget(this);
    Widget_chat = new ChatWidget(this);
    Widget_ManageStart = new ManageStartWidget(this);
    Widget_Setting = new SettingWidget(this);
    Widget_Todo = new todoWidget(this);
    ui->stackedWidget->insertWidget(0, Widget_SystemMonitor);
    ui->stackedWidget->addWidget(Widget_chat);
    ui->stackedWidget->addWidget(Widget_ManageStart);
    ui->stackedWidget->addWidget(Widget_Setting);
    ui->stackedWidget->addWidget(Widget_Todo);

    // 连接信号槽，切换页面
    connect(ui->pushButton_5, &QPushButton::clicked, [=]()
            { ui->stackedWidget->setCurrentWidget(Widget_SystemMonitor); });
    connect(ui->pushButton_3, &QPushButton::clicked, [=]()
            { ui->stackedWidget->setCurrentWidget(Widget_chat); });
    connect(ui->pushButton_4, &QPushButton::clicked, [=]()
            { ui->stackedWidget->setCurrentWidget(Widget_Todo); });
    connect(ui->pushButton_2, &QPushButton::clicked,
            [=]()
            { ui->stackedWidget->setCurrentWidget(Widget_ManageStart); });
    connect(ui->pushButton, &QPushButton::clicked, [=]()
            { ui->stackedWidget->setCurrentWidget(Widget_Setting); });
    // 设置初始页面
    ui->stackedWidget->setCurrentWidget(Widget_SystemMonitor);
    resize(1280, 720);
}

void mainWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event); // 保留基类调用（通常推荐）
    raise();
    activateWindow();
}

mainWidget::~mainWidget()
{
    delete ui;
}

void mainWidget::closeEvent(QCloseEvent *event)
{
    if (!Widget_ManageStart->isSaved)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Confirmation"),
                                      tr("您的更改尚未保存，是否要保存？"),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            Widget_ManageStart->saveMenuData();
        }
    }
    hide();
    event->ignore();
}
