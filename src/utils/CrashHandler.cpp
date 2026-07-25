
#include "CrashHandler.h"
#include <windows.h>
#include <dbghelp.h>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QDebug>

#pragma comment(lib, "dbghelp.lib")

static LONG WINAPI exceptionFilter(EXCEPTION_POINTERS *ep)
{
    QString dir = QCoreApplication::applicationDirPath() + "/log";
    QDir().mkpath(dir);

    QString path = dir + "/crash_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".dmp";

    HANDLE hFile = CreateFileW((LPCWSTR)path.utf16(), GENERIC_WRITE,
                               0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        qCritical() << "[CrashHandler] Failed to create dump file:" << path;
        return EXCEPTION_CONTINUE_SEARCH;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId          = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers    = FALSE;

    BOOL ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                                hFile, MiniDumpNormal, &mei, NULL, NULL);
    CloseHandle(hFile);

    if (ok)
    {
        qCritical() << "[CrashHandler] Minidump saved to" << path;
    }
    else
    {
        qCritical() << "[CrashHandler] MiniDumpWriteDump failed, error:" << GetLastError();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashHandler::install()
{
    SetUnhandledExceptionFilter(exceptionFilter);
    qDebug() << "[CrashHandler] Installed (dumps go to log/)";
}
