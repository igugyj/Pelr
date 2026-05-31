
#pragma once

#include <QWidget>
#include <QPainter>
#include <QColor>

class DiskUsageRing : public QWidget
{
    Q_OBJECT

public:
    explicit DiskUsageRing(QWidget *parent = nullptr);

    void setPercent(double percent);
    void setRingColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setCenterText(const QString &top, const QString &bottom);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_percent = 0.0;
    QColor m_ringColor = QColor("#4CAF50");
    QColor m_bgColor = QColor(60, 60, 60, 40);
    QString m_topText;
    QString m_bottomText;

    QColor interpolateColor() const;
};
