#include "StartupShortcut.h"
#include "ComApartment.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>

#include <shlobj.h>
#include <shobjidl.h>

#include <thread>

namespace
{
QString native(const QString &s)
{
    return QDir::toNativeSeparators(s);
}
} // namespace

QString StartupShortcut::startupDir()
{
    // 首选：已知文件夹 API，能正确处理重定向/漫游配置
    PWSTR p = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Startup, KF_FLAG_DEFAULT, nullptr, &p)) && p)
    {
        const QString dir = QDir::fromNativeSeparators(QString::fromWCharArray(p));
        CoTaskMemFree(p);
        if (QDir(dir).exists())
            return dir;
    }
    else if (p)
    {
        CoTaskMemFree(p);
    }

    // 回退：按环境变量拼（Qt 没有 StartupLocation）
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString appData = env.value("APPDATA");
    if (appData.isEmpty())
    {
        appData = env.value("USERPROFILE");
        if (appData.isEmpty())
            return QString();
        appData.append("/AppData/Roaming");
    }
    return appData + "/Microsoft/Windows/Start Menu/Programs/Startup";
}

QString StartupShortcut::shortcutPath()
{
    const QString dir = startupDir();
    if (dir.isEmpty())
        return QString();
    const QString exe = QCoreApplication::applicationFilePath();
    if (exe.isEmpty())
        return QString();
    return dir + "/" + QFileInfo(exe).baseName() + ".lnk";
}

bool StartupShortcut::exists()
{
    const QString path = shortcutPath();
    return !path.isEmpty() && QFile::exists(path);
}

bool StartupShortcut::create(const QString &exePath, HRESULT *outHr, const QString &args)
{
    if (outHr)
        *outHr = E_FAIL;
    if (exePath.isEmpty())
    {
        if (outHr)
            *outHr = E_INVALIDARG;
        return false;
    }

    const QString path = shortcutPath();
    if (path.isEmpty())
        return false;
    QDir().mkpath(startupDir());

    HRESULT hr = E_FAIL;
    // 独立 STA 线程：不受主线程已被初始化成 MTA 的影响（RPC_E_CHANGED_MODE 问题）
    std::thread worker([&]
                       {
        ComApartment apartment(COINIT_APARTMENTTHREADED);
        if (!apartment.usable())
        {
            hr = apartment.hr();
            return;
        }

        IShellLinkW *link = nullptr;
        IPersistFile *file = nullptr;

        hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW,
                              reinterpret_cast<void **>(&link));
        if (SUCCEEDED(hr) && link)
        {
            link->SetPath(native(exePath).toStdWString().c_str());
            link->SetWorkingDirectory(native(QFileInfo(exePath).absolutePath()).toStdWString().c_str());
            link->SetIconLocation(native(exePath).toStdWString().c_str(), 0);
            if (!args.isEmpty())
                link->SetArguments(args.toStdWString().c_str());

            hr = link->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&file));
            if (SUCCEEDED(hr) && file)
            {
                hr = file->Save(native(path).toStdWString().c_str(), TRUE);
                file->Release();
                file = nullptr;
            }
            link->Release();
            link = nullptr;
        }
    });
    worker.join(); // join 即同步：线程退出时已自行 CoUninitialize

    if (outHr)
        *outHr = hr;
    if (FAILED(hr))
        return false;
    return QFile::exists(path);
}

bool StartupShortcut::remove()
{
    const QString path = shortcutPath();
    if (path.isEmpty() || !QFile::exists(path))
        return false;
    return QFile::remove(path);
}

QString StartupShortcut::hresultToString(HRESULT hr)
{
    LPWSTR buffer = nullptr;
    const DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                     FORMAT_MESSAGE_IGNORE_INSERTS,
                                     nullptr, static_cast<DWORD>(hr),
                                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    QString text = (len && buffer) ? QString::fromWCharArray(buffer, static_cast<int>(len)).trimmed()
                                   : QString("unknown error");
    if (buffer)
        LocalFree(buffer);
    return text;
}
