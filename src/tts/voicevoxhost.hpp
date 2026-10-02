#pragma once

class IVoicevoxTts;

// voicevox 插件宿主：先预加载 voicevox_core.dll，再 QPluginLoader 载入插件。
namespace VoicevoxHost
{
// 幂等；不可用时返回 nullptr 并记日志。
IVoicevoxTts *acquire();
}
