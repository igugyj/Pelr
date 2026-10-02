#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QStringList>

// 可选组件的运行时布局。官方 VOICEVOX_CORE 包结构是规范布局，历史布局仍被
// 兼容识别，任一合法布局齐备即认为组件可用（各组件的候选表见下方注释）。
namespace ComponentPaths
{

inline QString exeDir()
{
    const QString dir = QCoreApplication::applicationDirPath();
    return dir.isEmpty() ? QDir::currentPath() : dir;
}

// ---- Live2D Cubism Core ----
inline QString live2dDir() { return exeDir() + QStringLiteral("/Live2D"); }
inline QString live2dCoreDll() { return live2dDir() + QStringLiteral("/Live2DCubismCore.dll"); }
inline QString live2dLicense() { return live2dDir() + QStringLiteral("/LICENSE.md"); }
inline QString live2dLegacyDll() { return exeDir() + QStringLiteral("/Live2DCubismCore.dll"); }

// ---- VOICEVOX CORE ----
inline QString voicevoxDir() { return exeDir() + QStringLiteral("/voicevox_core"); }
inline QString voicevoxDict() { return voicevoxDir() + QStringLiteral("/dict"); }
inline QString voicevoxModels() { return voicevoxDir() + QStringLiteral("/models"); }

// 官方 VOICEVOX_CORE 包的原样结构是规范布局（c_api/lib + onnxruntime/lib +
// dict + models），用户下载官方 zip 解压到 voicevox_core/ 下即为该结构。
// 其余两种只是兼容，顺序即优先级，规范布局必须排第一：
//   1. 官方包结构 voicevox_core/{c_api,onnxruntime}/lib/*.dll   ← 规范
//   2. 平铺 voicevox_core/*.dll
//   3. legacy：exe 目录根部（阶段 D 之前的构建输出，已废弃）
// 候选表是唯一事实源：voicevoxAvailable()、VoicevoxHost::preloadVoicevoxCore()、
// 插件 ensureOnnxRuntime() 三处共用，避免各自维护候选表而漂移。
inline QStringList voicevoxCoreDllCandidates()
{
    return {
        voicevoxDir() + QStringLiteral("/c_api/lib/voicevox_core.dll"),
        voicevoxDir() + QStringLiteral("/voicevox_core.dll"),
        exeDir() + QStringLiteral("/voicevox_core.dll"),
    };
}

inline QStringList voicevoxOnnxDllCandidates()
{
    return {
        voicevoxDir() + QStringLiteral("/onnxruntime/lib/voicevox_onnxruntime.dll"),
        voicevoxDir() + QStringLiteral("/voicevox_onnxruntime.dll"),
        exeDir() + QStringLiteral("/voicevox_onnxruntime.dll"),
    };
}

// ---- Qt 插件 ----
inline QString pluginsDir() { return exeDir() + QStringLiteral("/plugins"); }
inline QString voicevoxPluginDll() { return pluginsDir() + QStringLiteral("/local_voicevox.dll"); }

} // namespace ComponentPaths
