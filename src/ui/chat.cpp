
#include "chat.h"
#include "ui_chat.h"
#include <QtGui/QFontMetrics>
#include <QFont>
#include <QLabel>
#include <QDebug>
#include <QSizePolicy>
#include <QScrollBar>
#include "data.hpp"

ChatWidget::ChatWidget(QWidget *parent) : QWidget(parent), ui(new Ui::chat)
{
    ui->setupUi(this);
    currentY = 0;
    standardHeight = ui->scrollAreaWidgetContents->height();
    ui->label->setText(tr("Hello, welcome to Pelr!"));
    connect(ui->pushButton, &QPushButton::clicked, this, &ChatWidget::on_sendMsg);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &ChatWidget::on_sendMsg);
    connect(LlamaClient::instance(), &LlamaClient::textGenerated,
            this, &ChatWidget::onTextGenerated);
    connect(LlamaClient::instance(), &LlamaClient::errorOccurred,
            this, &ChatWidget::onErrorOccurred);
    connect(ui->pushButton_2, &QPushButton::clicked, [&]()
            {
                // 删除旧的内容部件
                delete ui->scrollAreaWidgetContents;
                // 创建新的内容部件
                ui->scrollAreaWidgetContents = new QWidget();
                ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
                currentY = 0;
                // 清空历史记录 AI
                LlamaClient::instance()->clearHistory(); });
}

void ChatWidget::addMessage(const QString &message, const bool &isAI)
{
    if (message.isEmpty())
        return;
    qDebug() << "[Chat] AddMessage:" << message;
    QWidget *p = ui->scrollAreaWidgetContents;
    int maxWidth = p->width() * 4 / 5;
    int margin = 10;
    int verticalSpacing = 10;
    QLabel *label = new QLabel(p);
    QFontMetrics font_metrics = QFontMetrics(label->font());
    int text_width = font_metrics.horizontalAdvance(message);
    // 动态宽度
    if (text_width <= maxWidth)
    {
        label->setMinimumWidth(text_width);
    }
    else
    {
        label->setMinimumWidth(maxWidth);
        label->setWordWrap(true);
    }
    // 位置
    label->move(margin, currentY);

    label->setText(message);
    label->setAlignment(Qt::AlignLeft);
    label->setStyleSheet(
        ("background-color: " + QString(isAI ? "lightgray" : "lightblue") +
         ";border-radius: 10px;padding: 10px;margin: 10px;"));
    label->setMaximumWidth(maxWidth);
    // 计算标签所需高度
    label->adjustSize();
    int labelHeight = label->height();
    // 高度
    currentY += labelHeight + verticalSpacing;
    // 更新容器的最小高度
    p->setMinimumHeight(currentY);
    QScrollBar *vbar = ui->scrollArea->verticalScrollBar();
    vbar->setValue(vbar->maximum());
    p->update();
    label->show();
}

void ChatWidget::on_sendMsg()
{
    QString message = ui->lineEdit->text();
    addMessage(message, false);
    ui->lineEdit->clear();
    if (message.isEmpty())
    {
        // QMessageBox::warning(this, tr("警告"), tr("消息不能为空！"));
        addMessage(tr("Message cannot be empty!"), true);
        return;
    };
    LlamaClient::instance()->generateTextAsync(message, ai_id);
}

void ChatWidget::onTextGenerated(const QString &text, const int &id)
{
    if (id != ai_id)
        return;
    qDebug() << "[Chat] TextGenerated:" << text;
    // 处理生成的文本
    addMessage(text, true);
}

void ChatWidget::onErrorOccurred(const QString &error, const int &id)
{
    if (id != ai_id)
        return;
    qDebug() << "[Chat] ErrorOccurred:" << error;
    // 处理错误
    addMessage(tr("错误：%1").arg(error), true);
}

void ChatWidget::retranslateUI()
{
    ui->label->setText(tr("Hello, welcome to Pelr!"));
}

ChatWidget::~ChatWidget()
{
    delete ui;
}
