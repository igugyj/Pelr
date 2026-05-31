#include "DiskUsageRing.hpp"
#include <QPainter>
#include <QPen>
#include <QFontMetrics>

DiskUsageRing::DiskUsageRing(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(160, 160);
    setMaximumSize(300, 300);
}

void DiskUsageRing::setPercent(double percent)
{
    m_percent = qBound(0.0, percent, 100.0);
    m_ringColor = interpolateColor();
    update();
}

void DiskUsageRing::setRingColor(const QColor &color)
{
    m_ringColor = color;
    update();
}

void DiskUsageRing::setBackgroundColor(const QColor &color)
{
    m_bgColor = color;
    update();
}

void DiskUsageRing::setCenterText(const QString &top, const QString &bottom)
{
    m_topText = top;
    m_bottomText = bottom;
    update();
}

QColor DiskUsageRing::interpolateColor() const
{
    if (m_percent < 60.0)
        return QColor("#4CAF50");
    if (m_percent < 80.0)
        return QColor("#FF9800");
    return QColor("#F44336");
}

void DiskUsageRing::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    int penWidth = side / 10;
    if (penWidth < 8) penWidth = 8;

    int margin = penWidth / 2 + 8;
    QRectF rect(margin, margin, side - margin * 2, side - margin * 2);

    // background ring
    p.setPen(QPen(m_bgColor, penWidth, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(rect, 0, 360 * 16);

    // foreground arc (start at 12 o'clock, clockwise)
    int startAngle = 90 * 16;
    int spanAngle = static_cast<int>(m_percent / 100.0 * 360 * 16);
    p.setPen(QPen(m_ringColor, penWidth, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(rect, startAngle, -spanAngle);

    // center text
    QFont font = p.font();
    font.setPixelSize(side / 7);
    font.setBold(true);
    p.setFont(font);
    p.setPen(QColor(220, 220, 220));

    QString pctText = QString::number(m_percent, 'f', 1) + "%";
    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(pctText);
    int textH = fm.height();
    p.drawText((width() - textW) / 2, (height() - textH) / 2 + fm.ascent(), pctText);

    // bottom text (smaller)
    if (!m_bottomText.isEmpty())
    {
        font.setPixelSize(side / 14);
        font.setBold(false);
        p.setFont(font);
        p.setPen(QColor(160, 160, 160));
        QFontMetrics fm2(font);
        int btmW = fm2.horizontalAdvance(m_bottomText);
        int btmY = (height() + textH / 2 + fm2.height());
        p.drawText((width() - btmW) / 2, btmY, m_bottomText);
    }
}
