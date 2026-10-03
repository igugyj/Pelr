#include <windows.h>               // ① 先提供基本类型
#include <initguid.h>              // ② 启用 GUID 定义模式
#include "AudioSpectrumDetector.h" // ③ 此时内部包含的 <mmdeviceapi.h> 会生成 GUID
#include <cmath>
#include <iostream>
#include <QDebug>
#include <future>
#include <memory>
#include "ComApartment.h"
// Windows 多媒体库链接
#pragma comment(lib, "ole32.lib")

AudioSpectrumDetector::AudioSpectrumDetector() {}

AudioSpectrumDetector::~AudioSpectrumDetector()
{
    stop();
}

bool AudioSpectrumDetector::start()
{
    if (m_running)
        return true;
    if (m_thread.joinable())
        return false; // 上一次 stop 尚未结束

    auto ready = std::make_shared<std::promise<bool>>();
    std::future<bool> result = ready->get_future();

    // 所有 COM/WASAPI 操作都在本线程内完成：
    // 主线程公寓不受影响，也不受主线程已被初始化成其它公寓的影响。
    m_thread = std::thread([this, ready]
                           {
        ComApartment apartment(COINIT_MULTITHREADED);
        if (!apartment.usable())
        {
            qDebug() << "[ASD] CoInitializeEx failed:" << Qt::hex << (unsigned long)apartment.hr();
            ready->set_value(false);
            return;
        }

        if (!initializeWasapi())
        {
            releaseResources();
            ready->set_value(false);
            return;
        }

        m_running = true;
        ready->set_value(true);

        if (m_running)
            captureThreadFunc();

        releaseResources(); // 本线程创建的 COM 对象在本线程释放
        m_running = false;
    });

    return result.get(); // 同步等待初始化结果
}

bool AudioSpectrumDetector::initializeWasapi()
{
    HRESULT hr;

    // 1. 枚举设备
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL,
                          IID_IMMDeviceEnumerator, (void **)&m_enumerator);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] CoCreateInstance enumerator failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    // 2. 获取默认音频渲染端点
    hr = m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] GetDefaultAudioEndpoint failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    // 3. 激活音频客户端
    hr = m_device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void **)&m_audioClient);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] Activate IAudioClient failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    // 4. 获取混音格式
    hr = m_audioClient->GetMixFormat(&m_waveFormat);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] GetMixFormat failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }
    m_sampleRate = (float)m_waveFormat->nSamplesPerSec;

    // 5. 初始化音频客户端（环回，共享）——使用 0 让系统自动分配缓冲区
    hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                   AUDCLNT_STREAMFLAGS_LOOPBACK,
                                   0, 0, m_waveFormat, nullptr);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] Initialize failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    // 6. 获取缓冲区大小
    hr = m_audioClient->GetBufferSize(&m_bufferFrameCount);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] GetBufferSize failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    // 7. 获取捕获客户端
    hr = m_audioClient->GetService(IID_IAudioCaptureClient, (void **)&m_captureClient);
    if (FAILED(hr))
    {
        qDebug() << "[ASD] GetService IAudioCaptureClient failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    // 8. 事件已移除，环回模式不使用事件驱动
    // m_hEvent 保持 nullptr，后续无需操作

    // 9. 准备 FFT
    m_fftCfg = kiss_fftr_alloc(FFT_SIZE, 0, nullptr, nullptr);
    m_fftInput.assign(FFT_SIZE, 0.0f);
    m_fftOutput.resize(FFT_SIZE / 2 + 1);
    m_window.resize(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i)
        m_window[i] = 0.5f * (1.0f - std::cos(2.0 * M_PI * i / (FFT_SIZE - 1)));

    m_inputPos = 0;

    hr = m_audioClient->Start();
    if (FAILED(hr))
    {
        qDebug() << "[ASD] AudioClient Start failed:" << Qt::hex << (unsigned long)hr;
        return false;
    }

    qDebug() << "[ASD] Initialization successful";
    return true;
}

void AudioSpectrumDetector::stop()
{
    m_running = false; // 采集循环 10ms 轮询该标志后退出

    if (m_thread.joinable())
        m_thread.join(); // 工作线程内自行释放 COM 资源并退出公寓

    m_running = false;
}

void AudioSpectrumDetector::releaseResources()
{
    if (m_audioClient)
        m_audioClient->Stop();

    if (m_fftCfg)
    {
        free(m_fftCfg);
        m_fftCfg = nullptr;
    }

    if (m_captureClient)
    {
        m_captureClient->Release();
        m_captureClient = nullptr;
    }
    if (m_audioClient)
    {
        m_audioClient->Release();
        m_audioClient = nullptr;
    }
    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }
    if (m_enumerator)
    {
        m_enumerator->Release();
        m_enumerator = nullptr;
    }
    if (m_waveFormat)
    {
        CoTaskMemFree(m_waveFormat);
        m_waveFormat = nullptr;
    }
    if (m_hEvent)
    {
        CloseHandle(m_hEvent);
        m_hEvent = nullptr;
    }
}

bool AudioSpectrumDetector::isAudioPlaying() const
{
    return m_isActive.load();
}

float AudioSpectrumDetector::currentEnergy() const
{
    return m_currentEnergy.load();
}

void AudioSpectrumDetector::captureThreadFunc()
{
    HRESULT hr;
    UINT32 packetLength = 0;
    BYTE *data;

    const bool isFloat = (m_waveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                         (m_waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                          reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_waveFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
    const int nChannels = m_waveFormat->nChannels;

    while (m_running)
    {
        // 改用 Sleep 轮询，避免事件驱动问题
        Sleep(10);

        hr = m_captureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr))
            break;

        while (packetLength > 0 && m_running)
        {
            UINT32 numFrames;
            DWORD flags;
            hr = m_captureClient->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);
            if (FAILED(hr))
                break;

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                for (UINT32 i = 0; i < numFrames; ++i)
                {
                    m_fftInput[m_inputPos] = 0.0f;
                    m_inputPos++;
                    if (m_inputPos >= HOP_SIZE)
                    {
                        processWindowAndFFT();
                        m_inputPos = 0;
                    }
                }
            }
            else
            {
                if (isFloat)
                {
                    float *samples = (float *)data;
                    for (UINT32 i = 0; i < numFrames; ++i)
                    {
                        m_fftInput[m_inputPos] = samples[i * nChannels];
                        m_inputPos++;
                        if (m_inputPos >= HOP_SIZE)
                        {
                            processWindowAndFFT();
                            m_inputPos = 0;
                        }
                    }
                }
                else
                {
                    short *samples = (short *)data;
                    const float scale = 1.0f / 32768.0f;
                    for (UINT32 i = 0; i < numFrames; ++i)
                    {
                        m_fftInput[m_inputPos] = samples[i * nChannels] * scale;
                        m_inputPos++;
                        if (m_inputPos >= HOP_SIZE)
                        {
                            processWindowAndFFT();
                            m_inputPos = 0;
                        }
                    }
                }
            }

            hr = m_captureClient->ReleaseBuffer(numFrames);
            if (FAILED(hr))
                break;

            hr = m_captureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr))
                break;
        }
    }
}

void AudioSpectrumDetector::processWindowAndFFT()
{
    std::vector<float> windowedInput(FFT_SIZE);
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        windowedInput[i] = m_fftInput[i] * m_window[i];
    }

    kiss_fftr(m_fftCfg, windowedInput.data(), m_fftOutput.data());

    float energy = 0.0f;
    int relevantBins = FFT_SIZE / 4;
    for (int i = 0; i < relevantBins; ++i)
    {
        float mag = std::sqrt(m_fftOutput[i].r * m_fftOutput[i].r +
                              m_fftOutput[i].i * m_fftOutput[i].i);
        energy += mag;
    }

    // 噪声门
    if (energy < m_noiseGateThreshold)
        energy = 0.0f;

    // 指数平滑
    m_smoothedEnergy = m_smoothedEnergy * (1.0f - m_smoothingFactor) + energy * m_smoothingFactor;

    m_currentEnergy.store(m_smoothedEnergy);
    m_isActive.store(m_smoothedEnergy > 0.01f); // 激活阈值

    // 移动滑动窗口
    for (int i = 0; i < FFT_SIZE - HOP_SIZE; ++i)
        m_fftInput[i] = m_fftInput[i + HOP_SIZE];
    m_inputPos = FFT_SIZE - HOP_SIZE;
}