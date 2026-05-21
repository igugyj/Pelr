
#include "MenuWidget.h"
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QStyle>
#include <QPainter>
#include <QDebug>
#include "data.hpp"

MenuWidget::MenuWidget(QWidget *parent) : QWidget(
                                              parent)
{
    setupUI();
    applyStyles();
    // 置顶
    if (DataManager::instance().getBasicData().isTop)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    }
    else
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    }
    // 设置窗口背景和圆角
    setAttribute(Qt::WA_TranslucentBackground);
}

void MenuWidget::showNearMouse()
{
    // 获取鼠标当前位置
    QPoint mousePos = QCursor::pos();

    // 获取屏幕尺寸
    QScreen *screen = QApplication::screenAt(mousePos);
    QRect screenGeometry = screen->geometry();

    // 距离鼠标的距离
    int distance = 60 * DataManager::instance().getBasicData().model_size / 150;

    // 获取窗口尺寸
    adjustSize();
    int width = this->width();
    int height = this->height();

    // 计算显示位置 - 优先显示在鼠标右侧
    int x = mousePos.x() + distance;
    int y = mousePos.y();

    // 检查右侧空间是否足够
    if (x + width > screenGeometry.right())
    {
        // 右侧空间不足，显示在左侧
        x = mousePos.x() - width - distance;
    }

    // 检查底部空间是否足够
    if (y + height > screenGeometry.bottom())
    {
        y = screenGeometry.bottom() - height;
    }

    // 确保窗口不会超出屏幕顶部
    if (y < screenGeometry.top())
    {
        y = screenGeometry.top();
    }

    // 确保窗口不会超出屏幕左侧
    if (x < screenGeometry.left())
    {
        x = screenGeometry.left();
    }
    qInfo() << "Menu show at: (" << x << "," << y << ")";

    // 移动窗口到计算位置
    move(x, y);

    // 显示窗口
    show();

    // 激活窗口
    activateWindow();
}

void MenuWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 绘制圆角矩形背景
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(45, 45, 48, 192));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

void MenuWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 标题
    QLabel *titleLabel = new QLabel(DataManager::instance().Project_Name);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);
}

void MenuWidget::applyStyles()
{
    setStyleSheet(R"(
            menuWidget {
                background: transparent;
            }
            QLabel#titleLabel {
                color: #e6e6e6;
                font-weight: bold;
                font-size: 14px;
                padding: 5px;
            }
            QPushButton {
                background-color: rgba(13,126,191,128);
                color: white;
                border: none;
                padding: 8px;
                border-radius: 5px;
                font-weight: bold;
                min-width: 120px;
            }
            QPushButton:hover {
                background-color: rgba(13,126,191,255);
            }
            QPushButton:pressed {
                background-color: rgba(192,243,255,196);
            }
        )");
}

void MenuWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        left_button_down = true;
        dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
}

void MenuWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (left_button_down && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - dragStartPos);
    }
}

void MenuWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 按住鼠标释放时停止拖动
    if (event->button() == Qt::LeftButton)
    {
        left_button_down = false;
    }
}

void MenuWidget::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
