
#pragma once

#include <QDebug>
#include <cstdint>

struct ProcessMemoryInfo
{
    qint64 workingSet = 0;       // physical memory (KB)
    qint64 peakWorkingSet = 0;   // peak physical memory (KB)
    qint64 privateBytes = 0;     // private committed memory (KB)
    qint64 virtualSize = 0;      // virtual address space (KB)
};

struct TotalMemoryInfo
{
    qint64 totalPhys = 0;        // total physical RAM (KB)
    qint64 availPhys = 0;        // available physical RAM (KB)
};

ProcessMemoryInfo getProcessMemoryInfo();
TotalMemoryInfo   getTotalMemoryInfo();
