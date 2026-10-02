#include "voicevoxhost.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QLibrary>
#include <QPluginLoader>
#include <QStringList>
#include <mutex>

#include "componentmanager.hpp"
#include "componentpaths.hpp"
#include "voicevoxiface.hpp"

namespace
{
// 插件的 import 表引用 voicevox_core.dll。voicevox_core/ 不在 Windows 默认
// DLL 搜索路径里，必须先按绝对路径把本体载入进程；Windows 解析插件 import
// 时按 base name 查已加载模块列表即可命中。
bool preloadVoicevoxCore(QLibrary **keepAlive)
{
    bool presentButFailed = false;

    // 逐候选尝试并逐条留日志：「文件不存在」和「文件在但依赖缺失」对排查的
    // 价值完全不同，不能混成一句含糊的 module not found。
    for (const QString &path : ComponentPaths::voicevoxCoreDllCandidates())
    {
        if (path.isEmpty())
            continue;

        QLibrary *lib = new QLibrary(path); // 故意泄漏：预加载的 DLL 须存活到进程结束
        if (lib->load())
        {
            qDebug().noquote() << "[VoicevoxHost] preloaded" << path;
            *keepAlive = lib;
            return true;
        }

        if (QFileInfo::exists(path))
        {
            presentButFailed = true;
            qWarning().noquote() << "[VoicevoxHost] present but not loadable:" << path
                                 << "-" << lib->errorString();
        }
        else
        {
            qDebug().noquote() << "[VoicevoxHost] not present:" << path;
        }
        delete lib;
    }

    // voicevox_core.dll 由 MSVC 构建，纯净 Windows 未装 VC++ Redistributable 时
    // MSVCP140/VCRUNTIME140 缺失，症状正是「文件在却加载不动」。
    if (presentButFailed)
        qWarning().noquote() << "[VoicevoxHost] voicevox_core.dll exists but failed to load;"
                             << "on a clean machine install the Microsoft Visual C++"
                             << "Redistributable, then retry";
    return false;
}
} // namespace

IVoicevoxTts *VoicevoxHost::acquire()
{
    // 三个 static 都故意泄漏：插件与预加载的 DLL 须存活到进程结束
    static QLibrary *s_preload = nullptr;
    static QPluginLoader *s_loader = nullptr;
    static IVoicevoxTts *s_iface = nullptr;
    static std::once_flag s_once;

    // 只 gate「插件文件存在 + DLL 可加载」，不 gate dict/models —— 否则设置页
    // 里用文件对话框选择辞书/模型的流程会被自己卡死。
    std::call_once(s_once, [&]()
                   {
        if (!ComponentManager::voicevoxPluginAvailable())
            return;
        if (!preloadVoicevoxCore(&s_preload))
            return;

        const QString path = ComponentPaths::voicevoxPluginDll();
        s_loader = new QPluginLoader(path);
        QObject *root = s_loader->instance();
        if (!root)
        {
            qWarning().noquote() << "[VoicevoxHost] plugin load failed:" << path
                                 << "-" << s_loader->errorString();
            delete s_loader;
            s_loader = nullptr;
            return;
        }

        s_iface = qobject_cast<IVoicevoxTts *>(root);
        if (!s_iface)
        {
            qWarning().noquote() << "[VoicevoxHost] plugin has no IVoicevoxTts interface:" << path;
            s_loader->unload();
            delete s_loader;
            s_loader = nullptr;
            return;
        }

        qDebug().noquote() << "[VoicevoxHost] plugin loaded:" << path; });

    return s_iface;
}
