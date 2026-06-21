
#include "tray.h"
#include <QImageReader>
#include <QDebug>

void TrayIcon::loadGifFrames(const QString &path)
{
    m_gifFrames.clear();
    m_gifIdx = 0;

    QImageReader reader(path);
    QImage firstImg;

    for (int i = 0; i < 500; ++i)
    {
        if (!reader.canRead())
            break;
        QImage img = reader.read();
        if (img.isNull())
            break;

        // Detect loop: if new frame matches first frame, stop
        if (i > 0 && firstImg == img)
            break;

        if (i == 0)
            firstImg = img;

        int delay = reader.nextImageDelay();
        if (delay <= 0)
            delay = 100;
        m_gifFrames.append({QPixmap::fromImage(img), delay});
    }

    qDebug() << "[Tray] Loaded" << m_gifFrames.size() << "GIF frames from" << path;
}

void TrayIcon::advanceGifFrame()
{
    if (m_gifFrames.isEmpty())
        return;

    m_gifIdx = (m_gifIdx + 1) % m_gifFrames.size();
    setIcon(QIcon(m_gifFrames[m_gifIdx].pixmap));
    m_gifTimer->setInterval(m_gifFrames[m_gifIdx].delayMs);
}
