
#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QGroupBox>
#include "DiskUsageRing.hpp"
#include "TranslationManager.h"

class SystemMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemMonitorWidget(QWidget *parent = nullptr);
    ~SystemMonitorWidget();

    void retranslateUI();

private:
    void setupUI();
    void refreshAll();
    void refreshDisk();
    void refreshMemory();
    QString formatBytes(qint64 bytes) const;

    // rings
    DiskUsageRing *m_ringMem;
    DiskUsageRing *m_ringDisk;
    QLabel *m_labelRingMem;
    QLabel *m_labelRingDisk;

    // disk labels
    QLabel *m_labelResources;
    QLabel *m_labelVoicevox;
    QLabel *m_labelTtsSize;
    QLabel *m_labelTtsPath;
    QLabel *m_labelLogSize;
    QLabel *m_labelLogPath;
    QLabel *m_labelUserSize;
    QLabel *m_labelUserPath;
    QLabel *m_labelTotalDisk;

    // memory labels
    QLabel *m_labelMemPhysical;
    QLabel *m_labelMemPeak;
    QLabel *m_labelMemPrivate;
    QLabel *m_labelMemVirtual;

    QLabel *m_titleLabel;
    QGroupBox *m_memGroup;
    QGroupBox *m_diskGroup;
    QLabel *m_labelTtsCache;
    QLabel *m_labelLog;
    QLabel *m_labelUserData;
    QPushButton *m_btnClearTts;
    QPushButton *m_btnClearLog;

    QTimer *m_timer;
};
