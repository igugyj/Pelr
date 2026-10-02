#include "componentmanager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>

#include "miniz.h"

#include "NotificationWidget.h"
#include "componentpaths.hpp"

namespace
{
// 提升为核心文件后必须保留在 Live2D/ 根目录的名字
const char *kSurvivorDll = "Live2DCubismCore.dll";
const char *kSurvivorLicense = "LICENSE.md";

bool fileUsable(const QString &path)
{
    const QFileInfo fi(path);
    return fi.exists() && fi.isFile() && fi.size() > 4096;
}

bool anyUsable(const QStringList &paths)
{
    for (const QString &p : paths)
        if (fileUsable(p))
            return true;
    return false;
}

QString firstUsable(const QStringList &paths)
{
    for (const QString &p : paths)
        if (fileUsable(p))
            return p;
    return {};
}
} // namespace

ComponentManager &ComponentManager::instance()
{
    // 故意泄漏：避免在 QCoreApplication 销毁之后再析构 QObject
    static ComponentManager *m = new ComponentManager;
    return *m;
}

ComponentManager::ComponentManager(QObject *parent) : QObject(parent) {}

bool ComponentManager::exists(const QString &path)
{
    return QFileInfo::exists(path);
}

QStringList ComponentManager::findZips(const QString &dir)
{
    QStringList out;
    const QFileInfoList list = QDir(dir).entryInfoList({QStringLiteral("*.zip")}, QDir::Files, QDir::Name);
    for (const QFileInfo &fi : list)
        out << fi.absoluteFilePath();
    return out;
}

// ---------------------------------------------------------------------------
// 探测
// ---------------------------------------------------------------------------
bool ComponentManager::live2dCoreAvailable()
{
    return fileUsable(ComponentPaths::live2dCoreDll())
        || fileUsable(ComponentPaths::live2dLegacyDll());
}

bool ComponentManager::voicevoxPluginAvailable()
{
    return fileUsable(ComponentPaths::voicevoxPluginDll());
}

bool ComponentManager::voicevoxAvailable()
{
    // 两类 DLL 各自独立在候选表里找，允许混合布局
    // （例如官方包的 c_api/lib + 本项目拷到 exe 根的 onnxruntime）。
    return anyUsable(ComponentPaths::voicevoxCoreDllCandidates())
        && anyUsable(ComponentPaths::voicevoxOnnxDllCandidates())
        && exists(ComponentPaths::voicevoxDict())
        && exists(ComponentPaths::voicevoxModels())
        && voicevoxPluginAvailable();
}

ComponentManager::Info ComponentManager::scanLive2d()
{
    Info info;
    info.id = QStringLiteral("live2d");
    info.name = tr("Live2D Cubism Core");
    info.dir = ComponentPaths::live2dDir();

    const QStringList zips = findZips(info.dir);
    if (!zips.isEmpty())
        info.zip = zips.first();

    if (live2dCoreAvailable())
    {
        info.state = State::Installed;
        info.detail = exists(ComponentPaths::live2dCoreDll())
                          ? ComponentPaths::live2dCoreDll()
                          : ComponentPaths::live2dLegacyDll();
    }
    else if (!info.zip.isEmpty())
    {
        info.state = State::NeedsExtract;
        info.detail = info.zip;
    }
    else if (QDir(info.dir).exists())
    {
        const QFileInfoList entries = QDir(info.dir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        info.state = entries.isEmpty() ? State::Missing : State::Invalid;
        info.detail = tr("Live2DCubismCore.dll not found in %1").arg(info.dir);
    }
    else
    {
        info.state = State::Missing;
        info.detail = tr("Directory not found: %1").arg(info.dir);
    }
    return info;
}

ComponentManager::Info ComponentManager::scanVoicevox()
{
    Info info;
    info.id = QStringLiteral("voicevox");
    info.name = tr("VOICEVOX CORE");
    info.dir = ComponentPaths::voicevoxDir();

    if (voicevoxAvailable())
    {
        info.state = State::Installed;
        info.detail = QFileInfo(firstUsable(ComponentPaths::voicevoxCoreDllCandidates()))
                          .absolutePath();
    }
    else
    {
        info.state = State::Missing;
        QStringList missing;
        const bool hasDll = anyUsable(ComponentPaths::voicevoxCoreDllCandidates());
        const bool hasOnnx = anyUsable(ComponentPaths::voicevoxOnnxDllCandidates());
        if (!hasDll || !hasOnnx)
            missing << tr("voicevox_core.dll / voicevox_onnxruntime.dll");
        if (!exists(ComponentPaths::voicevoxDict()))
            missing << tr("dict");
        if (!exists(ComponentPaths::voicevoxModels()))
            missing << tr("models");
        if (!voicevoxPluginAvailable())
            missing << QFileInfo(ComponentPaths::voicevoxPluginDll()).fileName();
        info.detail = tr("Missing: %1").arg(missing.join(QStringLiteral(", ")));
    }
    return info;
}

QList<ComponentManager::Info> ComponentManager::scanAll()
{
    return {scanLive2d(), scanVoicevox()};
}

// ---------------------------------------------------------------------------
// 解包
// ---------------------------------------------------------------------------
bool ComponentManager::unzip(const QString &zipPath, const QString &destDir,
                             QStringList *created, QString *err)
{
    if (created)
        created->clear();

    mz_zip_archive zip = {};
    // Windows 下 fopen 使用本地代码页，必须用 toLocal8Bit 而非 toUtf8
    if (!mz_zip_reader_init_file(&zip, zipPath.toLocal8Bit().constData(), 0))
    {
        if (err)
            *err = tr("Cannot open archive: %1").arg(zipPath);
        return false;
    }

    const mz_uint count = mz_zip_reader_get_num_files(&zip);

    // 若所有条目共享同一顶层目录（如 CubismSdkForNative-5-r.5/），解包时整体剥离
    QString prefix;
    bool uniform = true;
    for (mz_uint i = 0; i < count && uniform; ++i)
    {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&zip, i, &st))
        {
            uniform = false;
            break;
        }
        const QString name = QString::fromUtf8(st.m_filename);
        if (name.isEmpty())
        {
            uniform = false;
            break;
        }
        const int slash = name.indexOf(QLatin1Char('/'));
        if (slash < 0)
        {
            uniform = false; // 根级散文件
            break;
        }
        const QString top = (slash == name.size() - 1) ? name : name.left(slash + 1);
        if (prefix.isEmpty())
            prefix = top;
        else if (prefix != top)
            uniform = false;
    }
    if (!uniform)
        prefix.clear();

    QDir().mkpath(destDir);
    const QString destRoot = QDir(destDir).absolutePath();
    QSet<QString> createdSet;

    for (mz_uint i = 0; i < count; ++i)
    {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&zip, i, &st) || st.m_is_directory)
            continue;

        QString name = QString::fromUtf8(st.m_filename);
        if (!prefix.isEmpty())
        {
            if (!name.startsWith(prefix))
                continue;
            name = name.mid(prefix.size());
        }
        if (name.isEmpty())
            continue;
        if (name.contains(QStringLiteral("..")) || name.startsWith(QLatin1Char('/'))
            || name.contains(QStringLiteral(":/")))
        {
            if (err)
                *err = tr("Unsafe path in archive: %1").arg(name);
            mz_zip_reader_end(&zip);
            return false;
        }

        size_t size = 0;
        void *buf = mz_zip_reader_extract_to_heap(&zip, i, &size, 0);
        if (!buf)
        {
            if (err)
                *err = tr("Extract failed: %1").arg(name);
            mz_zip_reader_end(&zip);
            return false;
        }

        const QString outPath = destRoot + QLatin1Char('/') + name;
        QDir().mkpath(QFileInfo(outPath).absolutePath());

        QFile f(outPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            mz_free(buf);
            if (err)
                *err = tr("Cannot write: %1").arg(outPath);
            mz_zip_reader_end(&zip);
            return false;
        }
        const qint64 written = f.write(static_cast<const char *>(buf), qint64(size));
        f.close();
        mz_free(buf);
        if (written != qint64(size))
        {
            if (err)
                *err = tr("Short write: %1").arg(outPath);
            mz_zip_reader_end(&zip);
            return false;
        }

        const int slash = name.indexOf(QLatin1Char('/'));
        const QString top = slash < 0 ? name : name.left(slash);
        if (!createdSet.contains(top))
        {
            createdSet.insert(top);
            if (created)
                created->append(top);
        }
    }

    mz_zip_reader_end(&zip);
    return true;
}

// ---------------------------------------------------------------------------
// 校验 / 提升 / 清理
// ---------------------------------------------------------------------------
bool ComponentManager::validateLive2dLayout(const QString &dir)
{
    return fileUsable(dir + QLatin1Char('/') + kSurvivorDll);
}

bool ComponentManager::promoteLive2dArtifacts(const QString &live2dDir, QString *err)
{
    // 1) Core DLL：递归查找，优先 x86_64 路径，其次取体积最大者
    QString found;
    qint64 foundSize = -1;
    bool foundPreferred = false;

    QDirIterator it(live2dDir, QStringList{QStringLiteral("Live2DCubismCore.dll")},
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        const QString p = it.next();
        const QFileInfo fi(p);
        if (!fi.exists() || fi.size() < 4096)
            continue;
        const bool preferred = p.contains(QStringLiteral("x86_64"));
        if (found.isEmpty()
            || (preferred && !foundPreferred)
            || (preferred == foundPreferred && fi.size() > foundSize))
        {
            found = p;
            foundSize = fi.size();
            foundPreferred = preferred;
        }
    }
    if (found.isEmpty())
    {
        if (err)
            *err = tr("Live2DCubismCore.dll not found in extracted content");
        return false;
    }

    const QString dllTarget = ComponentPaths::live2dCoreDll();
    if (QFileInfo::exists(found) && QFileInfo(found).absoluteFilePath() != QFileInfo(dllTarget).absoluteFilePath())
    {
        if (QFileInfo::exists(dllTarget))
            QFile::remove(dllTarget);
        if (!QFile::rename(found, dllTarget) && !QFile::copy(found, dllTarget))
        {
            if (err)
                *err = tr("Cannot move %1 to %2").arg(found, dllTarget);
            return false;
        }
    }

    // 2) LICENSE：优先 SDK 根 LICENSE.md，退回 Core/LICENSE.md
    const QString licTarget = ComponentPaths::live2dLicense();
    const QStringList candidates = {
        live2dDir + QStringLiteral("/LICENSE.md"),
        live2dDir + QStringLiteral("/Core/LICENSE.md"),
    };
    for (const QString &src : candidates)
    {
        const QFileInfo fi(src);
        if (!fi.exists() || !fi.isFile())
            continue;
        if (fi.absoluteFilePath() == QFileInfo(licTarget).absoluteFilePath())
            break;
        if (QFileInfo::exists(licTarget))
            QFile::remove(licTarget);
        if (QFile::copy(src, licTarget))
            break;
    }

    return true;
}

void ComponentManager::cleanupAfterExtract(const QString &live2dDir, const QStringList &created)
{
    if (!fileUsable(ComponentPaths::live2dCoreDll()))
        return; // 安全阀：目标 DLL 未就位时不清理任何东西

    for (const QString &top : created)
    {
        if (top.isEmpty())
            continue;
        const QString path = live2dDir + QLatin1Char('/') + top;
        const QFileInfo fi(path);
        if (!fi.exists())
            continue;
        if (fi.isDir())
        {
            QDir(path).removeRecursively();
            continue;
        }
        if (top == QLatin1String(kSurvivorDll) || top == QLatin1String(kSurvivorLicense))
            continue;
        QFile::remove(path);
    }
}

// ---------------------------------------------------------------------------
// 对外动作
// ---------------------------------------------------------------------------
bool ComponentManager::scanAllForZip()
{
    const Info info = scanLive2d();
    if (info.state != State::NeedsExtract)
        return false; // 已就位或无 zip：不重复解包，避免每次启动都提示

    const QString dir = ComponentPaths::live2dDir();
    const QStringList zips = findZips(dir);
    if (zips.isEmpty())
        return false;

    bool allOk = true;
    QString error;
    QStringList allCreated;

    for (const QString &zipPath : zips)
    {
        QStringList created;
        if (!unzip(zipPath, dir, &created, &error))
        {
            allOk = false;
            break;
        }
        allCreated += created;
        if (!promoteLive2dArtifacts(dir, &error))
        {
            allOk = false;
            break;
        }
        // 落地校验通过才允许清理残留
        if (!validateLive2dLayout(dir))
        {
            error = tr("Extracted layout is invalid: %1 not found").arg(ComponentPaths::live2dCoreDll());
            allOk = false;
            break;
        }
        qInfo() << "[Components] extracted" << zipPath;
    }

    if (allOk)
    {
        cleanupAfterExtract(dir, allCreated);
        NotificationWidget::showNotification(tr("Components"),
                                             tr("Live2D Cubism Core extracted to %1").arg(dir),
                                             5000, NotificationWidget::Information);
    }
    else
    {
        qWarning() << "[Components] extraction failed:" << error;
        NotificationWidget::showNotification(tr("Components"), error, 5000, NotificationWidget::Warning);
    }

    emit instance().componentsChanged();
    return allOk;
}

void ComponentManager::deleteZips(const QString &dir)
{
    const QStringList zips = findZips(dir);
    if (zips.isEmpty())
        return;
    int failed = 0;
    for (const QString &z : zips)
    {
        if (!QFile::remove(z))
        {
            ++failed;
            qWarning() << "[Components] cannot remove" << z;
        }
    }
    if (failed == 0)
        qInfo() << "[Components] removed" << zips.size() << "archive(s) from" << dir;
    emit instance().componentsChanged();
}
