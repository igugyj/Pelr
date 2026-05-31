#include "systemInfo.hpp"

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <QFile>
#include <QRegularExpression>
#endif

ProcessMemoryInfo getProcessMemoryInfo()
{
    ProcessMemoryInfo info;

#ifdef Q_OS_WIN
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        info.workingSet = static_cast<qint64>(pmc.WorkingSetSize / 1024);
        info.peakWorkingSet = static_cast<qint64>(pmc.PeakWorkingSetSize / 1024);
        info.privateBytes = static_cast<qint64>(pmc.PagefileUsage / 1024);
        info.virtualSize = static_cast<qint64>(pmc.PagefileUsage / 1024);
    }
    else
    {
        qWarning() << "[SystemInfo] GetProcessMemoryInfo failed";
    }
#else
    // fallback: read /proc/self/status on Linux
    QFile proc("/proc/self/status");
    if (proc.open(QIODevice::ReadOnly))
    {
        while (!proc.atEnd())
        {
            QString line = QString::fromUtf8(proc.readLine());
            if (line.startsWith("VmRSS:"))
                info.workingSet = line.section('\t', 1).trimmed().chopped(3).toLongLong();
            else if (line.startsWith("VmPeak:"))
                info.peakWorkingSet = line.section('\t', 1).trimmed().chopped(3).toLongLong();
            else if (line.startsWith("VmSize:"))
                info.virtualSize = line.section('\t', 1).trimmed().chopped(3).toLongLong();
        }
    }
#endif
    return info;
}

TotalMemoryInfo getTotalMemoryInfo()
{
    TotalMemoryInfo info;
#ifdef Q_OS_WIN
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms))
    {
        info.totalPhys = static_cast<qint64>(ms.ullTotalPhys / 1024);
        info.availPhys = static_cast<qint64>(ms.ullAvailPhys / 1024);
    }
    else
    {
        qWarning() << "[SystemInfo] GlobalMemoryStatusEx failed";
    }
#else
    QFile meminfo("/proc/meminfo");
    if (meminfo.open(QIODevice::ReadOnly))
    {
        while (!meminfo.atEnd())
        {
            QString line = QString::fromUtf8(meminfo.readLine());
            if (line.startsWith("MemTotal:"))
                info.totalPhys = line.section(QRegularExpression("\\s+"), 1).trimmed().chopped(2).toLongLong();
            else if (line.startsWith("MemAvailable:"))
                info.availPhys = line.section(QRegularExpression("\\s+"), 1).trimmed().chopped(2).toLongLong();
        }
    }
#endif
    return info;
}
