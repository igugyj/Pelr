
#include "tray.h"
#include <QPainter>
#include <QApplication>
#include "data.hpp"

void TrayIcon::textRotate()
{
    const auto &cfg = DataManager::instance().getBasicData();

    m_textAngle = (m_textAngle + 30) % 360;

    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        if (m_textRotating)
        {
            painter.translate(32, 32);
            painter.rotate(m_textAngle);
            painter.translate(-32, -32);
        }

        QFont font(qApp->font());
        font.setPointSize(64);
        font.setBold(true);
        painter.setFont(font);

        painter.setPen(QColor(
            m_textRotating ? cfg.color_tray.background : cfg.color_tray.forground));

        painter.drawText(pixmap.rect(), Qt::AlignCenter, cfg.music_tray_symbol);
    }

    setIcon(QIcon(pixmap));
}
