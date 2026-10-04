#ifndef AUDIOSPECTRUMDETECTOR_H
#define AUDIOSPECTRUMDETECTOR_H

#include <atomic>
#include <thread>
#include <vector>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "kiss_fftr.h"

class AudioSpectrumDetector
{
public:
    AudioSpectrumDetector();
    ~AudioSpectrumDetector();

    bool start();                // 初始化并启动后台检测
    void stop();                 // 停止检测并释放资源
    bool isAudioPlaying() const; // 查询当前是否有音频活动
    float currentEnergy() const; // 当前平滑后的频谱能量

private:
    bool initializeWasapi();   // WASAPI/COM 初始化（只能在工作线程内调用）
    void captureThreadFunc();  // 后台采集与 FFT 处理线程
    void releaseResources();   // 释放 COM/FFT 资源（只能在工作线程内调用）
    void processWindowAndFFT();

    // COM/WASAPI 接口
    IMMDeviceEnumerator *m_enumerator = nullptr;
    IMMDevice *m_device = nullptr;
    IAudioClient *m_audioClient = nullptr;
    IAudioCaptureClient *m_captureClient = nullptr;
    WAVEFORMATEX *m_waveFormat = nullptr;
    HANDLE m_hEvent = nullptr;
    UINT32 m_bufferFrameCount = 0;

    // FFT 参数
    static constexpr int FFT_SIZE = 2048;
    static constexpr int HOP_SIZE = 512; // 每次新样本数
    kiss_fftr_cfg m_fftCfg = nullptr;
    std::vector<float> m_fftInput;         // 滑动窗口缓冲区，长度 FFT_SIZE
    int m_inputPos = 0;                    // 当前写入位置
    std::vector<kiss_fft_cpx> m_fftOutput; // FFT_SIZE/2+1
    std::vector<float> m_window;           // 汉宁窗系数
    float m_sampleRate = 0;

    // 频谱能量处理
    float m_smoothedEnergy = 0.0f;
    float m_smoothingFactor = 0.3f;      // 指数平滑系数
    float m_noiseGateThreshold = 0.1f;   // 噪声门阈值（可根据需要调整）
    std::atomic<bool> m_isActive{false};
    std::atomic<float> m_currentEnergy{0.0f};

    // 线程控制
    std::atomic<bool> m_running{false};
    std::thread m_thread;
};

#endif // AUDIOSPECTRUMDETECTOR_H