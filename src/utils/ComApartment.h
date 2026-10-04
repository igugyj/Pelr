#pragma once

#include <windows.h>

// COM 公寓 RAII：谁初始化谁释放。
// usable() 同时接受 RPC_E_CHANGED_MODE（该线程已被别的组件初始化为另一种公寓，
// COM 依然可用，但不是我们初始化的，析构时不释放）。
class ComApartment
{
public:
    explicit ComApartment(DWORD model = COINIT_APARTMENTTHREADED)
    {
        m_hr = CoInitializeEx(nullptr, model);
        m_owned = SUCCEEDED(m_hr);
    }
    ~ComApartment()
    {
        if (m_owned)
            CoUninitialize();
    }

    ComApartment(const ComApartment &) = delete;
    ComApartment &operator=(const ComApartment &) = delete;

    bool usable() const { return SUCCEEDED(m_hr) || m_hr == RPC_E_CHANGED_MODE; }
    HRESULT hr() const { return m_hr; }

private:
    HRESULT m_hr = S_OK;
    bool m_owned = false;
};
