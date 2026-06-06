
#include "UpdateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include "NotificationWidget.h"
#include <iostream>

UpdateDialog::UpdateDialog(const VersionCheckSummary &summary, QWidget *parent)
    : QDialog(parent), m_summary(summary)
{
    setWindowTitle(tr("Check for Updates"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(600, 450);

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &) { retranslateUI(); });

    // 收集成功的源
    for (const auto &res : summary.results)
    {
        if (res.success)
        {
            m_successfulSources.append(res);
        }
    }

    setupUi();

    if (m_successfulSources.isEmpty())
    {
        m_statusLabel->setText(tr("Unable to retrieve update information."));
        m_downloadButton->setEnabled(false);
    }
    else
    {
        // 默认选择第一个源
        updateDetailsForSource(m_successfulSources.first());
        m_sourceComboBox->setCurrentIndex(0);
    }
    // 提示音效
    std::cout << "\a";
}

UpdateDialog::~UpdateDialog()
{
}

void UpdateDialog::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);

    // 本地版本信息
    QHBoxLayout *localLayout = new QHBoxLayout();
    m_labelCurrentVersion = new QLabel(tr("Current version:"));
    localLayout->addWidget(m_labelCurrentVersion);
    m_localVersionLabel = new QLabel(m_summary.localVersion);
    m_localVersionLabel->setStyleSheet("font-weight: bold;");
    localLayout->addWidget(m_localVersionLabel);
    localLayout->addStretch();
    m_mainLayout->addLayout(localLayout);

    // 状态标签
    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);
    if (m_summary.anyNewerVersion)
    {
        m_statusLabel->setText(tr("New version available!"));
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
    else if (m_summary.anySuccess)
    {
        m_statusLabel->setText(tr("You are using the latest version."));
    }
    else
    {
        m_statusLabel->setText(tr("Check failed. Please try again later."));
        m_statusLabel->setStyleSheet("color: red;");
    }
    m_mainLayout->addWidget(m_statusLabel);

    // 源选择下拉框
    QHBoxLayout *sourceLayout = new QHBoxLayout();
    m_labelSource = new QLabel(tr("Source:"));
    sourceLayout->addWidget(m_labelSource);
    m_sourceComboBox = new QComboBox();
    for (const auto &src : m_successfulSources)
    {
        QString display = QString("%1 (%2)").arg(src.sourceName, src.latestVersion);
        m_sourceComboBox->addItem(display);
    }
    // 如果所有源都失败，添加一个占位项
    if (m_successfulSources.isEmpty())
    {
        m_sourceComboBox->addItem(tr("No successful source"));
    }
    sourceLayout->addWidget(m_sourceComboBox);
    sourceLayout->addStretch();
    m_mainLayout->addLayout(sourceLayout);

    connect(m_sourceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UpdateDialog::onSourceSelectionChanged);

    // 发布说明浏览器
    m_labelReleaseNotes = new QLabel(tr("Release Notes:"));
    m_mainLayout->addWidget(m_labelReleaseNotes);
    m_bodyBrowser = new QTextBrowser();
    m_bodyBrowser->setOpenExternalLinks(true);
    m_bodyBrowser->setPlaceholderText(tr("No release notes available."));
    m_mainLayout->addWidget(m_bodyBrowser, 1);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_downloadButton = new QPushButton(tr("Download"));
    m_downloadButton->setEnabled(!m_successfulSources.isEmpty());
    connect(m_downloadButton, &QPushButton::clicked, this, &UpdateDialog::onDownloadClicked);
    buttonLayout->addWidget(m_downloadButton);

    m_closeButton = new QPushButton(tr("Close"));
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);

    m_mainLayout->addLayout(buttonLayout);
}

void UpdateDialog::retranslateUI()
{
    setWindowTitle(tr("Check for Updates"));
    m_labelCurrentVersion->setText(tr("Current version:"));
    m_labelSource->setText(tr("Source:"));
    m_labelReleaseNotes->setText(tr("Release Notes:"));
    if (m_summary.anyNewerVersion)
        m_statusLabel->setText(tr("New version available!"));
    else if (m_summary.anySuccess)
        m_statusLabel->setText(tr("You are using the latest version."));
    else
        m_statusLabel->setText(tr("Check failed. Please try again later."));
    m_bodyBrowser->setPlaceholderText(tr("No release notes available."));
    m_downloadButton->setText(tr("Download"));
    m_closeButton->setText(tr("Close"));
}

void UpdateDialog::onSourceSelectionChanged(int index)
{
    if (index < 0 || index >= m_successfulSources.size())
        return;
    updateDetailsForSource(m_successfulSources.at(index));
}

void UpdateDialog::updateDetailsForSource(const SourceCheckResult &result)
{
    // 显示发布说明（Markdown 纯文本，QTextBrowser 可渲染部分富文本）
    m_bodyBrowser->setMarkdown(result.body.isEmpty() ? tr("No release notes provided.") : result.body);

    // 可选的：在状态栏附近显示具体版本
    QString status = QString("%1 version: %2").arg(result.sourceName, result.latestVersion);
    if (!result.publishedAt.isEmpty())
    {
        status += QString(" (%1)").arg(result.publishedAt);
    }
    // 可以在某处显示，这里不重复添加控件，仅通过标题或tooltip
    setToolTip(status);
}

void UpdateDialog::onDownloadClicked()
{
    int index = m_sourceComboBox->currentIndex();
    if (index < 0 || index >= m_successfulSources.size())
    {
        NotificationWidget::showNotification(
            tr("Download"), tr("No download source available."),
            5000, NotificationWidget::Warning);
        return;
    }

    const SourceCheckResult &result = m_successfulSources.at(index);
    if (result.downloadUrl.isEmpty())
    {
        NotificationWidget::showNotification(
            tr("Download"), tr("No download URL provided by this source."),
            5000, NotificationWidget::Warning);
        return;
    }

    bool success = QDesktopServices::openUrl(QUrl(result.downloadUrl));
    if (!success)
    {
        NotificationWidget::showNotification(
            tr("Download"), tr("Failed to open download URL."),
            5000, NotificationWidget::Warning);
    }
}
