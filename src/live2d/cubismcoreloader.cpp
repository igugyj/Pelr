#include "cubismcoreloader.hpp"

#include <QDebug>
#include <QStringList>

#include "componentpaths.hpp"

#define PELR_CUBISM_CORE_DEF(name) QFunctionPointer p_##name = nullptr;
PELR_CUBISM_CORE_FUNCS(PELR_CUBISM_CORE_DEF)
#undef PELR_CUBISM_CORE_DEF

#define PELR_CUBISM_CORE_CLEAR(name) p_##name = nullptr;

CubismCoreLoader &CubismCoreLoader::instance()
{
    static CubismCoreLoader s_instance;
    return s_instance;
}

bool CubismCoreLoader::load()
{
    if (m_ok)
        return true;

    const QStringList candidates = {
        ComponentPaths::live2dCoreDll(),
        ComponentPaths::live2dLegacyDll(),
        QStringLiteral("Live2DCubismCore.dll"),
    };

    for (const QString &path : candidates)
    {
        if (path.isEmpty())
            continue;

        if (m_lib.isLoaded())
            m_lib.unload();
        m_lib.setFileName(path);

        if (!m_lib.load())
        {
            qDebug().noquote() << "[Core] not loadable:" << path << "-" << m_lib.errorString();
            continue;
        }

        if (!resolveAll())
        {
            qDebug().noquote() << "[Core] exports incomplete in" << path;
            clearAll();
            m_lib.unload();
            continue;
        }

        m_ok = true;
        qDebug().noquote() << "[Core] Live2D Cubism Core loaded from" << path;
        return true;
    }

    qWarning().noquote() << "[Core] Live2D Cubism Core not installed. Tried:" << candidates;
    return false;
}

bool CubismCoreLoader::resolveAll()
{
    bool ok = true;

#define PELR_CUBISM_CORE_RESOLVE(name)        \
    p_##name = m_lib.resolve(#name);          \
    if (!p_##name)                            \
        ok = false;
    PELR_CUBISM_CORE_FUNCS(PELR_CUBISM_CORE_RESOLVE)
#undef PELR_CUBISM_CORE_RESOLVE

    return ok;
}

void CubismCoreLoader::clearAll()
{
    PELR_CUBISM_CORE_FUNCS(PELR_CUBISM_CORE_CLEAR)
}
