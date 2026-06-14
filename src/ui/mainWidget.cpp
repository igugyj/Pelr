
#include "mainWidget.h"
#include "ui_mainWidget.h"
#include <QCloseEvent>
#include <QFile>
#include <QIcon>
#include <QDebug>
#include <QMessageBox>
#include "TranslationManager.h"
#if __has_include(<FluentUI3Style/fluentui3styleproperties.h>)
#include <FluentUI3Style/fluentui3styleproperties.h>
#endif
mainWidget::mainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::mainWidget)
{
    ui->setupUi(this);
    initUI();
    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &)
            { retranslateUI(); });
    connect(Widget_Setting, &SettingWidget::styleChanged, this, [this](const QString &style)
            { setSwitchStyleProperty(style); });
}

void mainWidget::setSwitchStyleProperty(const QString &style)
{
    if (style == "FluentUI3")
    {
        auto setSwitchStyle = [](QWidget *w)
        {
            for (auto *cb : w->findChildren<QCheckBox *>())
                cb->setProperty(SwitchStyleProperty, true);
        };
        setSwitchStyle(Widget_Setting);
        setSwitchStyle(Widget_Todo);
        qDebug() << "[MainWidget] Applied FluentUI3 SwitchStyleProperty";
    }
}

void mainWidget::retranslateUI()
{
    ui->retranslateUi(this);
}

void mainWidget::initUI()
{
    this->setWindowIcon(qApp->windowIcon());
    this->setWindowTitle(qApp->applicationName());

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
    // 手动应用主题效果
    setSwitchStyleProperty(DataManager::instance().getBasicData().theme);
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
                                      tr("Your changes have not been saved. Do you want to save?"),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            Widget_ManageStart->saveMenuData();
        }
    }
    hide();
    event->ignore();
}
