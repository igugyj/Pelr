#pragma once

#include <QString>

#include <windows.h>

// Windows 开机自启快捷方式管理
// （%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\<exe>.lnk）。
// ShellLink 是 COM 对象，主线程公寓状态不可控，create() 一律在独立 STA 线程内完成。
namespace StartupShortcut
{
// Startup 目录；取不到返回空串
QString startupDir();
// 完整快捷方式路径 <startupDir>/<exe basename>.lnk；取不到返回空串
QString shortcutPath();
bool exists();
// exePath 为程序本体路径；outHr 可空，失败时带回真实 HRESULT
bool create(const QString &exePath, HRESULT *outHr = nullptr, const QString &args = QString());
// 文件不存在或删除失败返回 false
bool remove();
// FormatMessage 生成的可读 HRESULT 文本
QString hresultToString(HRESULT hr);
}
