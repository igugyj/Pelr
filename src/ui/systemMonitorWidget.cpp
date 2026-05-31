#include "systemMonitorWidget.hpp"
#include "systemInfo.hpp"
#include "storageInfo.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

SystemMonitorWidget::SystemMonitorWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    connect(m_btnClearTts, &QPushButton::clicked, this, [this]()
            {
        StorageInfoData info = StorageManager::getStorageInfo();
        if (StorageManager::clearDirContents(info.ttsCachePath))
            refreshDisk(); });

    connect(m_btnClearLog, &QPushButton::clicked, this, [this]()
            {
        StorageInfoData info = StorageManager::getStorageInfo();
        if (StorageManager::clearDirContents(info.logPath))
            refreshDisk(); });

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &SystemMonitorWidget::refreshAll);
    refreshAll();
    m_timer->start();
}

SystemMonitorWidget::~SystemMonitorWidget()
{
    m_timer->stop();
}

void SystemMonitorWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->setSpacing(12);

    // ── header ──
    QLabel *titleLabel = new QLabel(tr("系统监控"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // ── two rings ──
    QHBoxLayout *ringRow = new QHBoxLayout();
    ringRow->setSpacing(32);
    ringRow->addStretch();

    // memory ring column
    QVBoxLayout *memCol = new QVBoxLayout();
    memCol->setAlignment(Qt::AlignCenter);
    memCol->setSpacing(8);
    m_ringMem = new DiskUsageRing(this);
    m_ringMem->setFixedSize(180, 180);
    memCol->addWidget(m_ringMem, 0, Qt::AlignCenter);
    m_labelRingMem = new QLabel(tr("内存占用"));
    m_labelRingMem->setAlignment(Qt::AlignCenter);
    QFont lblFont = m_labelRingMem->font();
    lblFont.setPointSize(11);
    m_labelRingMem->setFont(lblFont);
    m_labelRingMem->setStyleSheet("color: #aaa;");
    memCol->addWidget(m_labelRingMem);
    ringRow->addLayout(memCol);

    // disk ring column
    QVBoxLayout *diskCol = new QVBoxLayout();
    diskCol->setAlignment(Qt::AlignCenter);
    diskCol->setSpacing(8);
    m_ringDisk = new DiskUsageRing(this);
    m_ringDisk->setFixedSize(180, 180);
    diskCol->addWidget(m_ringDisk, 0, Qt::AlignCenter);
    m_labelRingDisk = new QLabel(tr("磁盘占用"));
    m_labelRingDisk->setAlignment(Qt::AlignCenter);
    m_labelRingDisk->setFont(lblFont);
    m_labelRingDisk->setStyleSheet("color: #aaa;");
    diskCol->addWidget(m_labelRingDisk);
    ringRow->addLayout(diskCol);

    ringRow->addStretch();
    mainLayout->addLayout(ringRow);

    // ── memory group ──
    QGroupBox *memGroup = new QGroupBox(tr("程序内存"));
    QFormLayout *memForm = new QFormLayout(memGroup);
    memForm->setSpacing(6);

    m_labelMemPhysical = new QLabel("--");
    m_labelMemPeak = new QLabel("--");
    m_labelMemPrivate = new QLabel("--");
    m_labelMemVirtual = new QLabel("--");

    memForm->addRow(tr("物理内存:"), m_labelMemPhysical);
    memForm->addRow(tr("峰值内存:"), m_labelMemPeak);
    memForm->addRow(tr("私有内存:"), m_labelMemPrivate);
    memForm->addRow(tr("虚拟内存:"), m_labelMemVirtual);
    mainLayout->addWidget(memGroup);

    // ── disk group ──
    QGroupBox *diskGroup = new QGroupBox(tr("磁盘空间"));
    QVBoxLayout *diskLayout = new QVBoxLayout(diskGroup);
    diskLayout->setSpacing(6);

    m_labelTotalDisk = new QLabel("--");
    QFont boldFont = m_labelTotalDisk->font();
    boldFont.setBold(true);
    m_labelTotalDisk->setFont(boldFont);
    diskLayout->addWidget(m_labelTotalDisk);

    QFormLayout *diskForm = new QFormLayout();
    diskForm->setSpacing(4);
    diskForm->setContentsMargins(0, 4, 0, 0);

    // Resources (contains only Live2D model files)
    m_labelResources = new QLabel("--");
    diskForm->addRow("Resources(Live2D):", m_labelResources);

    // voicevox_core (sibling of Resources, not indented)
    m_labelVoicevox = new QLabel("--");
    diskForm->addRow("voicevox_core:", m_labelVoicevox);

    // TTS cache
    QHBoxLayout *ttsRow = new QHBoxLayout();
    m_labelTtsSize = new QLabel("--");
    m_labelTtsPath = new QLabel();
    m_labelTtsPath->setStyleSheet("color: #888; font-size: 11px;");
    m_btnClearTts = new QPushButton(tr("清除"));
    m_btnClearTts->setFixedSize(50, 24);
    ttsRow->addWidget(m_labelTtsSize);
    ttsRow->addWidget(m_labelTtsPath);
    ttsRow->addStretch();
    ttsRow->addWidget(m_btnClearTts);
    QLabel *ttsLabel = new QLabel(tr("TTS 缓存:"));
    QHBoxLayout *ttsOuter = new QHBoxLayout();
    ttsOuter->addWidget(ttsLabel);
    ttsOuter->addLayout(ttsRow);
    diskForm->addRow(ttsOuter);

    // Log
    QHBoxLayout *logRow = new QHBoxLayout();
    m_labelLogSize = new QLabel("--");
    m_labelLogPath = new QLabel();
    m_labelLogPath->setStyleSheet("color: #888; font-size: 11px;");
    m_btnClearLog = new QPushButton(tr("清空"));
    m_btnClearLog->setFixedSize(50, 24);
    logRow->addWidget(m_labelLogSize);
    logRow->addWidget(m_labelLogPath);
    logRow->addStretch();
    logRow->addWidget(m_btnClearLog);
    QLabel *logLabel = new QLabel(tr("日志:"));
    QHBoxLayout *logOuter = new QHBoxLayout();
    logOuter->addWidget(logLabel);
    logOuter->addLayout(logRow);
    diskForm->addRow(logOuter);

    // User data
    m_labelUserSize = new QLabel("--");
    m_labelUserPath = new QLabel();
    m_labelUserPath->setStyleSheet("color: #888; font-size: 11px;");
    QHBoxLayout *userRow = new QHBoxLayout();
    userRow->addWidget(m_labelUserSize);
    userRow->addWidget(m_labelUserPath);
    userRow->addStretch();
    QLabel *userLabel = new QLabel(tr("用户数据:"));
    QHBoxLayout *userOuter = new QHBoxLayout();
    userOuter->addWidget(userLabel);
    userOuter->addLayout(userRow);
    diskForm->addRow(userOuter);

    diskLayout->addLayout(diskForm);
    mainLayout->addWidget(diskGroup);

    mainLayout->addStretch();
}

void SystemMonitorWidget::refreshAll()
{
    refreshMemory();
    refreshDisk();
}

void SystemMonitorWidget::refreshMemory()
{
    ProcessMemoryInfo proc = getProcessMemoryInfo();
    TotalMemoryInfo total = getTotalMemoryInfo();

    double memPct = 0.0;
    if (total.totalPhys > 0)
        memPct = 100.0 * proc.workingSet / total.totalPhys;

    m_ringMem->setPercent(memPct);
    m_ringMem->setCenterText(
        QString::number(memPct, 'f', 1) + "%",
        formatBytes(proc.workingSet * 1024));
    m_labelRingMem->setText(QString("%1  %2 / %3")
                                .arg(tr("内存"))
                                .arg(formatBytes(proc.workingSet * 1024))
                                .arg(formatBytes(total.totalPhys * 1024)));

    m_labelMemPhysical->setText(formatBytes(proc.workingSet * 1024));
    m_labelMemPeak->setText(formatBytes(proc.peakWorkingSet * 1024));
    m_labelMemPrivate->setText(formatBytes(proc.privateBytes * 1024));
    m_labelMemVirtual->setText(formatBytes(proc.virtualSize * 1024));
}

void SystemMonitorWidget::refreshDisk()
{
    StorageInfoData info = StorageManager::getStorageInfo();

    double diskPct = info.usagePercent();
    m_ringDisk->setPercent(diskPct);
    m_ringDisk->setCenterText(
        QString::number(diskPct, 'f', 1) + "%",
        formatBytes(info.runningDirSize));
    m_labelRingDisk->setText(QString("%1  %2 / %3")
                                 .arg(tr("磁盘"))
                                 .arg(formatBytes(info.runningDirSize))
                                 .arg(formatBytes(info.diskTotal)));

    m_labelTotalDisk->setText(
        tr("总占用量: %1 / %2  (可用: %3)")
            .arg(formatBytes(info.totalAppSize))
            .arg(formatBytes(info.diskTotal))
            .arg(formatBytes(info.diskAvailable)));

    m_labelResources->setText(formatBytes(info.resourcesSize));
    m_labelVoicevox->setText(formatBytes(info.voicevoxCoreSize));

    m_labelTtsSize->setText(formatBytes(info.ttsCacheSize));
    m_labelTtsPath->setText("(" + info.ttsCachePath + ")");
    m_labelLogSize->setText(formatBytes(info.logSize));
    m_labelLogPath->setText("(" + info.logPath + ")");
    m_labelUserSize->setText(formatBytes(info.userDataSize));
    m_labelUserPath->setText("(" + info.userDataPath + ")");
}

QString SystemMonitorWidget::formatBytes(qint64 bytes) const
{
    if (bytes < 1024)
        return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024LL * 1024 * 1024)
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}
