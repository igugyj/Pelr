#include "globalinputlistener.h"

GlobalInputListener &GlobalInputListener::instance()
{
    static GlobalInputListener inst;
    return inst;
}

GlobalInputListener::~GlobalInputListener()
{
    stopListening();
}

bool GlobalInputListener::isListening() const
{
    return m_isListening;
}

bool GlobalInputListener::startListening()
{
    if (!m_keyboardHook)
    {
        m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc, GetModuleHandle(nullptr), 0);
    }
    if (!m_mouseHook)
    {
        m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProc, GetModuleHandle(nullptr), 0);
    }
    m_isListening = true;
    return (m_keyboardHook != nullptr && m_mouseHook != nullptr);
}

void GlobalInputListener::stopListening()
{
    if (m_keyboardHook)
    {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = nullptr;
    }
    if (m_mouseHook)
    {
        UnhookWindowsHookEx(m_mouseHook);
        m_mouseHook = nullptr;
    }
    m_isListening = false;
}

ModifierKeys GlobalInputListener::getCurrentModifiers()
{
    ModifierKeys mods = NoModifier;
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        mods |= Shift;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
        mods |= Ctrl;
    if (GetAsyncKeyState(VK_MENU) & 0x8000)
        mods |= Alt;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000))
        mods |= Win;
    return mods;
}

LRESULT CALLBACK GlobalInputListener::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        auto *kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        int vkCode = kbdStruct->vkCode;

        // 我们不在修饰键按下时单独发信号，而是等待组合键
        // if (!isModifierKeyCode(vkCode)) {
        emit instance().keyPressed(vkCode, getCurrentModifiers());
        // }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK GlobalInputListener::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        auto *msStruct = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
        MouseButton btn = UnknownButton;

        if (wParam == WM_LBUTTONUP)
            btn = LeftButton;
        else if (wParam == WM_RBUTTONUP)
            btn = RightButton;
        else if (wParam == WM_MBUTTONUP)
            btn = MiddleButton;
        else if (wParam == WM_XBUTTONUP)
        {
            btn = (GET_XBUTTON_WPARAM(msStruct->mouseData) == XBUTTON1) ? XButton1 : XButton2;
        }

        if (btn != UnknownButton)
        {
            emit instance().mouseReleased(btn, msStruct -> pt.x, msStruct->pt.y, getCurrentModifiers());
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool GlobalInputListener::switchListener()
{
    if (isListening())
        stopListening();
    else
        startListening();
    return m_isListening;
}
