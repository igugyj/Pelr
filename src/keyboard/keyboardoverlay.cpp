#include "keyboardoverlay.h"
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QScreen>
#include <QFontMetrics>
#include  "data.hpp"

KeyboardOverlay::KeyboardOverlay(QWidget *parent) : QWidget(parent) {
    // 设置为透明、穿透、无边框置顶工具窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_TranslucentBackground);

    // 覆盖主屏幕
    QRect screenGeom = QApplication::primaryScreen()->geometry();
    setGeometry(screenGeom);

    connect(&m_masterTimer, &QTimer::timeout, this, &KeyboardOverlay::tick);
    m_masterTimer.start(DT_MS);
}

void KeyboardOverlay::onNormalKey(const QString &text) {
    if (text.isEmpty()) return;
    if (m_specialMode) {
        clearAllTiles();
        m_specialMode = false;
    }
    addTile(text, false);
}

void KeyboardOverlay::onSpecialKey(const QString &text) {
    if (text.isEmpty()) return;
    clearAllTiles();
    m_specialMode = true;
    addTile(text, true);
}

void KeyboardOverlay::addTile(const QString &text, bool isSpecial) {
    TileEntry entry;
    entry.text = text;
    entry.isSpecial = isSpecial;
    entry.state = TileEntry::Appearing;
    entry.opacity = 0.0;
    entry.lifetimeMs = isSpecial ? SPECIAL_TIMEOUT : NORMAL_TIMEOUT;

    // 初始化在靠右滑入的起始点
    entry.currentX = width() / 2.0 + 200;
    entry.targetX = 0; // 会在 recomputeTargets 中更新

    m_entries.append(entry);
    if (m_entries.size() > MAX_TILES) {
        // 强制移除最老的活跃格子
        for (auto &e: m_entries) {
            if (e.state != TileEntry::Disappearing) {
                e.state = TileEntry::Disappearing;
                break;
            }
        }
    }
    recomputeTargets();
}

void KeyboardOverlay::clearAllTiles() {
    for (auto &e: m_entries) {
        e.state = TileEntry::Disappearing;
    }
}

void KeyboardOverlay::recomputeTargets() {
    QFont font = qApp->font();
    font.setBold(true);
    font.setPointSize(20);
    QFontMetrics fm(font);

    // 计算所有非消失状态格子的总宽度
    int totalWidth = 0;
    int activeCount = 0;
    QList<int> widths;

    for (const auto &e: m_entries) {
        if (e.state != TileEntry::Disappearing) {
            int w = qMax(60, fm.horizontalAdvance(e.text) + 40);
            widths.append(w);
            totalWidth += w;
            activeCount++;
        } else {
            widths.append(0);
        }
    }

    if (activeCount == 0) return;
    totalWidth += (activeCount - 1) * TILE_SPACING;

    // 居中起始 X 坐标
    qreal startX = (width() - totalWidth) / 2.0;

    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].state != TileEntry::Disappearing) {
            m_entries[i].targetX = startX;
            startX += widths[i] + TILE_SPACING;
        } else {
            // 消失的格子滑向左侧
            m_entries[i].targetX = m_entries[i].currentX - 50;
        }
    }
}

void KeyboardOverlay::tick() {
    m_globalRainbowAngle += 2.0;
    if (m_globalRainbowAngle >= 360) m_globalRainbowAngle -= 360;

    bool needsRepaint = false;
    for (int i = 0; i < m_entries.size(); ++i) {
        auto &e = m_entries[i];
        needsRepaint = true;

        // X 坐标平滑逼近 (LERP)
        e.currentX += (e.targetX - e.currentX) * 0.2;

        if (e.state == TileEntry::Appearing) {
            e.opacity += 0.1;
            if (e.opacity >= 1.0) {
                e.opacity = 1.0;
                e.state = TileEntry::Active;
            }
        } else if (e.state == TileEntry::Active) {
            e.lifetimeMs -= DT_MS;
            if (e.lifetimeMs <= 0) {
                e.state = TileEntry::Disappearing;
                e.targetX = e.currentX - 100; // 向左滑出
            }
        } else if (e.state == TileEntry::Disappearing) {
            e.opacity -= 0.05;
        }
    }

    // 清理透明度降为 0 的格子
    m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                                   [](const TileEntry &e) {
                                       return e.state == TileEntry::Disappearing && e.opacity <= 0.0;
                                   }), m_entries.end());

    if (needsRepaint || !m_entries.isEmpty()) {
        update();
        raise();
    }
}

void KeyboardOverlay::paintEvent(QPaintEvent *) {
    if (m_entries.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto &e: m_entries) {
        drawTile(painter, e, m_globalRainbowAngle);
    }
}

void KeyboardOverlay::drawTile(QPainter &painter, const TileEntry &entry, qreal rainbowAngle) {
    painter.save();
    painter.setOpacity(entry.opacity);

    QFont font = qApp->font();
    font.setBold(true);
    font.setPointSize(20);
    painter.setFont(font);
    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(entry.text);
    int tileW = qMax(60, textW + 40);

    // 计算 Y 坐标：停靠在屏幕底部偏上
    qreal y = height() - TILE_HEIGHT - 100;
    QRectF rect(entry.currentX, y, tileW, TILE_HEIGHT);

    // 1. 绘制彩虹发光外框 (稍微大一圈)
    QRectF borderRect = rect.adjusted(-2, -2, 2, 2);
    QConicalGradient borderGrad(borderRect.center(), rainbowAngle);
    borderGrad.setColorAt(0.0, QColor::fromHsv(0, 255, 255));
    borderGrad.setColorAt(0.33, QColor::fromHsv(120, 255, 255));
    borderGrad.setColorAt(0.66, QColor::fromHsv(240, 255, 255));
    borderGrad.setColorAt(1.0, QColor::fromHsv(0, 255, 255));

    painter.setBrush(borderGrad);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(borderRect, 10, 10);

    // 2. 绘制内层背景色 (特殊键橙红，普通键暗蓝)
    QColor bgColor = entry.isSpecial ? QColor(240, 80, 20, 220) : QColor(30, 40, 60, 220);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect, 8, 8);

    // 3. 绘制文字
    painter.setPen(Qt::white);
    painter.drawText(rect, Qt::AlignCenter, entry.text);

    painter.restore();
}
