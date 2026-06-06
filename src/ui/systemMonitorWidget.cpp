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

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &)
            { retranslateUI(); });

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
    m_titleLabel = new QLabel(tr("System Monitor"));
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

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
    m_labelRingMem = new QLabel(tr("Memory Usage"));
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
    m_labelRingDisk = new QLabel(tr("Disk Usage"));
    m_labelRingDisk->setAlignment(Qt::AlignCenter);
    m_labelRingDisk->setFont(lblFont);
    m_labelRingDisk->setStyleSheet("color: #aaa;");
    diskCol->addWidget(m_labelRingDisk);
    ringRow->addLayout(diskCol);

    ringRow->addStretch();
    mainLayout->addLayout(ringRow);

    // ── memory group ──
    m_memGroup = new QGroupBox(tr("Process Memory"));
    QFormLayout *memForm = new QFormLayout(m_memGroup);
    memForm->setSpacing(6);

    m_labelMemPhysical = new QLabel("--");
    m_labelMemPeak = new QLabel("--");
    m_labelMemPrivate = new QLabel("--");
    m_labelMemVirtual = new QLabel("--");

    memForm->addRow(tr("Physical Memory:"), m_labelMemPhysical);
    memForm->addRow(tr("Peak Memory:"), m_labelMemPeak);
    memForm->addRow(tr("Private Memory:"), m_labelMemPrivate);
    memForm->addRow(tr("Virtual Memory:"), m_labelMemVirtual);
    mainLayout->addWidget(m_memGroup);

    // ── disk group ──
    m_diskGroup = new QGroupBox(tr("Disk Space"));
    QVBoxLayout *diskLayout = new QVBoxLayout(m_diskGroup);
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
    diskForm->addRow(tr("Resources (Live2D):"), m_labelResources);

    // voicevox_core (sibling of Resources, not indented)
    m_labelVoicevox = new QLabel("--");
    diskForm->addRow(tr("voicevox_core:"), m_labelVoicevox);

    // TTS cache
    QHBoxLayout *ttsRow = new QHBoxLayout();
    m_labelTtsSize = new QLabel("--");
    m_labelTtsPath = new QLabel();
    m_labelTtsPath->setStyleSheet("color: #888; font-size: 11px;");
    m_btnClearTts = new QPushButton(tr("Clear"));
    ttsRow->addWidget(m_labelTtsSize);
    ttsRow->addWidget(m_labelTtsPath);
    ttsRow->addStretch();
    ttsRow->addWidget(m_btnClearTts);
    m_labelTtsCache = new QLabel(tr("TTS Cache:"));
    QHBoxLayout *ttsOuter = new QHBoxLayout();
    ttsOuter->addWidget(m_labelTtsCache);
    ttsOuter->addLayout(ttsRow);
    diskForm->addRow(ttsOuter);

    // Log
    QHBoxLayout *logRow = new QHBoxLayout();
    m_labelLogSize = new QLabel("--");
    m_labelLogPath = new QLabel();
    m_labelLogPath->setStyleSheet("color: #888; font-size: 11px;");
    m_btnClearLog = new QPushButton(tr("Clear All"));
    m_btnClearLog->setFixedSize(50, 24);
    logRow->addWidget(m_labelLogSize);
    logRow->addWidget(m_labelLogPath);
    logRow->addStretch();
    logRow->addWidget(m_btnClearLog);
    m_labelLog = new QLabel(tr("Log:"));
    QHBoxLayout *logOuter = new QHBoxLayout();
    logOuter->addWidget(m_labelLog);
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
    m_labelUserData = new QLabel(tr("User Data:"));
    QHBoxLayout *userOuter = new QHBoxLayout();
    userOuter->addWidget(m_labelUserData);
    userOuter->addLayout(userRow);
    diskForm->addRow(userOuter);

    diskLayout->addLayout(diskForm);
    mainLayout->addWidget(m_diskGroup);

    mainLayout->addStretch();
}

void SystemMonitorWidget::retranslateUI()
{
    m_titleLabel->setText(tr("System Monitor"));
    m_memGroup->setTitle(tr("Process Memory"));
    m_diskGroup->setTitle(tr("Disk Space"));
    m_labelRingMem->setText(tr("Memory Usage"));
    m_labelRingDisk->setText(tr("Disk Usage"));
    m_btnClearTts->setText(tr("Clear"));
    m_btnClearTts->adjustSize();
    m_btnClearLog->setText(tr("Clear All"));
    m_btnClearLog->adjustSize();
    m_labelTtsCache->setText(tr("TTS Cache:"));
    m_labelLog->setText(tr("Log:"));
    m_labelUserData->setText(tr("User Data:"));

    auto updateFormLabel = [](QLayout *layout, QWidget *field, const QString &text)
    {
        if (auto *form = qobject_cast<QFormLayout *>(layout))
            if (auto *lbl = qobject_cast<QLabel *>(form->labelForField(field)))
                lbl->setText(text);
    };
    updateFormLabel(m_memGroup->layout(), m_labelMemPhysical, tr("Physical Memory:"));
    updateFormLabel(m_memGroup->layout(), m_labelMemPeak, tr("Peak Memory:"));
    updateFormLabel(m_memGroup->layout(), m_labelMemPrivate, tr("Private Memory:"));
    updateFormLabel(m_memGroup->layout(), m_labelMemVirtual, tr("Virtual Memory:"));
    updateFormLabel(m_diskGroup->layout(), m_labelResources, tr("Resources (Live2D):"));
    updateFormLabel(m_diskGroup->layout(), m_labelVoicevox, tr("voicevox_core:"));
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
                                .arg(tr("Memory"))
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
                                 .arg(tr("Disk"))
                                 .arg(formatBytes(info.runningDirSize))
                                 .arg(formatBytes(info.diskTotal)));

    m_labelTotalDisk->setText(
        tr("Total Usage: %1 / %2 (Available: %3)")
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
