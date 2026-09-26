#pragma once
#include <QObject>
#include <windows.h>
#include "convertcodetostring.h"

class GlobalInputListener : public QObject {
    Q_OBJECT

public:
    static GlobalInputListener &instance();

    bool startListening();

    void stopListening();

    bool isListening() const;

    bool switchListener();

signals:
    void keyPressed(int keyCode, ModifierKeys modifiers);

    void mouseReleased(MouseButton button, int x, int y, ModifierKeys modifiers);

private:
    GlobalInputListener() = default;

    ~GlobalInputListener();

    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    static ModifierKeys getCurrentModifiers();

    HHOOK m_keyboardHook = nullptr;
    HHOOK m_mouseHook = nullptr;
    bool m_isListening = false;
};
