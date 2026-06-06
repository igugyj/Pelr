
#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include "VersionChecker.h"
#include "TranslationManager.h"

class QTextBrowser;
class QPushButton;
class QComboBox;
class QLabel;
class QVBoxLayout;

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(const VersionCheckSummary &summary, QWidget *parent = nullptr);

    ~UpdateDialog();

    void retranslateUI();

private slots:
    void onDownloadClicked();

    void onSourceSelectionChanged(int index);

private:
    void setupUi();

    void updateDetailsForSource(const SourceCheckResult &result);

    VersionCheckSummary m_summary;
    QList<SourceCheckResult> m_successfulSources; // 只保留成功的源

    QLabel *m_localVersionLabel;
    QLabel *m_statusLabel;
    QLabel *m_labelCurrentVersion;
    QLabel *m_labelSource;
    QLabel *m_labelReleaseNotes;
    QComboBox *m_sourceComboBox;
    QTextBrowser *m_bodyBrowser;
    QPushButton *m_downloadButton;
    QPushButton *m_closeButton;
    QVBoxLayout *m_mainLayout;
};

#endif // UPDATEDIALOG_H
