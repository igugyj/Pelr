
#pragma once
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPair>
#include <QFuture>
#include <QtConcurrent>
#include "NotificationWidget.h"

static QFuture<void> launchByPathAsync(const QString &path)
{
    qDebug() << "[Launcher] Launching: " << path;
    return QtConcurrent::run([path]() -> bool
                             {
        try {
            bool success = false;
            // 对于URL，使用QDesktopServices
            if (path.startsWith("http://") || path.startsWith("https://") ||
                path.startsWith("ftp://") || path.startsWith("file://")) {
                success = QDesktopServices::openUrl(QUrl(path));
            }
            // 对于本地文件/文件夹，使用QDesktopServices
            else {
                QFileInfo fileInfo(path);
                if (fileInfo.exists()) {
                    // 只对.exe文件使用QProcess启动以设置工作目录
                    QString suffix = fileInfo.suffix().toLower();
                    if (fileInfo.isFile() && suffix == "exe") {
                        QProcess *process = new QProcess();
                        process->setWorkingDirectory(fileInfo.absolutePath());
                        process->setProgram(fileInfo.absoluteFilePath());

                        // 连接finished信号以便清理
                        QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                                         process, &QProcess::deleteLater);
                        QObject::connect(process, &QProcess::errorOccurred,
                                         [path, process](QProcess::ProcessError error) {
                                              qWarning() << "[Launcher] Process error for" << path << ":" << error;
                                             process->deleteLater();
                                         });

                        process->startDetached();
                        success = true;
                    } else {
                        // 其他所有文件（包括.lnk、.bat等）使用原来的方式
                        QUrl localUrl = QUrl::fromLocalFile(QDir::toNativeSeparators(path));
                        success = QDesktopServices::openUrl(localUrl);
                    }
                } else {
                    qWarning() << "[Launcher] File or directory does not exist:" << path;
                    QMetaObject::invokeMethod(qApp, [path]() {
                        NotificationWidget::showNotification(
                            QObject::tr("Warning"),
                            QObject::tr("File or directory does not exist: %1").arg(path),
                            5000,
                            NotificationWidget::Warning
                        );
                    }, Qt::QueuedConnection);
                    return false;
                }
            }
            if (!success) {
                qWarning() << "[Launcher] Failed to open:" << path;
                QMetaObject::invokeMethod(qApp, [path]() {
                    NotificationWidget::showNotification(
                        QObject::tr("Warning"),
                        QObject::tr("Failed to open: %1").arg(path),
                        5000,
                        NotificationWidget::Warning
                    );
                }, Qt::QueuedConnection);
                return false;
            }
            return true;
        } catch (const std::exception &e) {
            qCritical() << "[Launcher] Exception occurred while launching" << path << ":" << e.what();
            QMetaObject::invokeMethod(qApp, [path, e]() {
                NotificationWidget::showNotification(
                    QObject::tr("Error"),
                    QObject::tr("Exception occurred while launching: %1\n%2").arg(path).arg(e.what()),
                    5000, NotificationWidget::Critical
                );
            }, Qt::QueuedConnection);
            return false;
        } catch (...) {
            qCritical() << "[Launcher] Unknown exception occurred while launching" << path;
            QMetaObject::invokeMethod(qApp, [path]() {
                NotificationWidget::showNotification(
                    QObject::tr("Error"),
                    QObject::tr("Unknown exception occurred while launching: %1").arg(path),
                    5000,
                    NotificationWidget::Critical
                );
            }, Qt::QueuedConnection);
            return false;
        } });
}

// 同步版本
static void launchByPath(const QString &path)
{
    launchByPathAsync(path);
}
