/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppPal.hpp"
#include <windows.h>
#include <cstdio>
#include <stdarg.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Model/CubismMoc.hpp>
#include "LAppDefine.hpp"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <QElapsedTimer>
#include <QDebug>
using std::endl;
using namespace Csm;
using namespace std;
using namespace LAppDefine;

double LAppPal::s_currentFrame = 0.0;
double LAppPal::s_lastFrame = 0.0;
double LAppPal::s_deltaTime = 0.0;
#ifdef CSM_FIXED_FRAME_RATE
int LAppPal::s_frame = 0;
#endif

csmByte *LAppPal::LoadFileAsBytes(const string filePath, csmSizeInt *outSize)
{
    wchar_t wideStr[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0U, filePath.c_str(), -1, wideStr, MAX_PATH);

    int size = 0;
    struct _stat statBuf;
    if (_wstat(wideStr, &statBuf) == 0)
    {
        size = statBuf.st_size;
        if (size == 0)
        {
            if (DebugLogEnable)
            {
                PrintLogLn("Stat succeeded but file size is zero. path:%s", filePath.c_str());
            }
            return NULL;
        }
    }
    else
    {
        if (DebugLogEnable)
        {
            PrintLogLn("Stat failed. errno:%d path:%s", errno, filePath.c_str());
        }
        return NULL;
    }

    // 使用 _wfopen 打开宽字符路径的文件
    FILE *file = _wfopen(wideStr, L"rb");
    if (!file)
    {
        if (DebugLogEnable)
        {
            PrintLogLn("File open failed. path:%s", filePath.c_str());
        }
        return NULL;
    }

    // 分配内存并读取文件内容
    csmByte *buffer = static_cast<csmByte *>(CSM_MALLOC(size));
    if (buffer == NULL)
    {
        if (DebugLogEnable)
        {
            PrintLogLn("Memory allocation failed.");
        }
        fclose(file);
        return NULL;
    }

    size_t readSize = fread(buffer, 1, size, file);
    fclose(file);

    if (readSize != size)
    {
        if (DebugLogEnable)
        {
            PrintLogLn("File read error. path:%s", filePath.c_str());
        }
        CSM_FREE(buffer);
        return NULL;
    }

    *outSize = size;
    return buffer;
}

// csmByte *LAppPal::LoadFileAsBytes(const string filePath, csmSizeInt *outSize) {
//     wchar_t wideStr[MAX_PATH];
//     MultiByteToWideChar(CP_UTF8, 0U, filePath.c_str(), -1, wideStr, MAX_PATH);
//
//     int size = 0;
//     struct _stat statBuf;
//     if (_wstat(wideStr, &statBuf) == 0) {
//         size = statBuf.st_size;
//
//         if (size == 0) {
//             if (DebugLogEnable) {
//                 PrintLogLn("Stat succeeded but file size is zero. path:%s", filePath.c_str());
//             }
//             return NULL;
//         }
//     } else {
//         if (DebugLogEnable) {
//             PrintLogLn("Stat failed. errno:%d path:%s", errno, filePath.c_str());
//         }
//         return NULL;
//     }
//
//     std::wfstream file;
//     file.open(wideStr, std::ios::in | std::ios::binary);
//     if (!file.is_open()) {
//         if (DebugLogEnable) {
//             PrintLogLn("File open failed. path:%s", filePath.c_str());
//         }
//         return NULL;
//     }
//
//     // ファイル名はワイド文字で探しているがファイルの中身はutf-8なので、1バイトずつ取得する。
//
//     *outSize = size;
//     csmChar *buf = new char[*outSize];
//     std::wfilebuf *fileBuf = file.rdbuf();
//     for (csmUint32 i = 0; i < *outSize; i++) {
//         buf[i] = fileBuf->sbumpc();
//     }
//     file.close();
//
//     return reinterpret_cast<csmByte *>(buf);
// }

void LAppPal::ReleaseBytes(csmByte *byteData)
{
    delete[] byteData;
}

csmFloat32 LAppPal::GetDeltaTime()
{
    return static_cast<csmFloat32>(s_deltaTime);
}

void LAppPal::UpdateTime()
{
    static QElapsedTimer timer;
    static bool initialized = false;
    if (!initialized)
    {
        timer.start();
        initialized = true;
    }
    s_currentFrame = timer.elapsed() / 1000.0; // 毫秒转秒
    s_deltaTime = s_currentFrame - s_lastFrame;
    s_lastFrame = s_currentFrame;
}

void LAppPal::PrintLog(const csmChar *format, ...)
{
    va_list args;
    csmChar buf[256];
    va_start(args, format);
    vsnprintf_s(buf, sizeof(buf), format, args); // 標準出力でレンダリング

    // #ifdef CSM_DEBUG_MEMORY_LEAKING
    //     // メモリリークチェック時は大量の標準出力がはしり重いのでprintfを利用する
    //     std::printf(buf);
    // #else
    //     std::cout << buf;
    // #endif
    va_end(args);
    qDebug().noquote() << buf;
}

void LAppPal::PrintLogLn(const Csm::csmChar *format, ...)
{
    va_list args;
    csmChar buf[256];
    va_start(args, format);
    vsnprintf_s(buf, sizeof(buf), format, args); // 標準出力でレンダリング

    // #ifdef CSM_DEBUG_MEMORY_LEAKING
    //     // メモリリークチェック時は大量の標準出力がはしり重いのでprintfを利用する
    //     std::printf("%s\n", buf);
    // #else
    //     std::cout << buf << std::endl;
    // #endif
    va_end(args);
    qDebug().noquote() << buf;
}

void LAppPal::PrintMessage(const csmChar *message)
{
    PrintLog("%s", message);
}

void LAppPal::PrintMessageLn(const csmChar *message)
{
    PrintLogLn("%s", message);
}

bool LAppPal::ConvertMultiByteToWide(const csmChar *multiByte, wchar_t *wide, int wideSize)
{
    return MultiByteToWideChar(CP_UTF8, 0U, multiByte, -1, wide, wideSize) != 0;
}

bool LAppPal::ConvertWideToMultiByte(const wchar_t *wide, csmChar *multiByte, int multiByteSize)
{
    return WideCharToMultiByte(CP_UTF8, 0U, wide, -1, multiByte, multiByteSize, NULL, NULL) != 0;
}
