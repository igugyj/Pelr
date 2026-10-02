#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "LAppAllocator_Common.hpp"
#include "GLCore.h"

class LAppView;
class LAppTextureManager;

class LAppDelegate
{
public:
    static LAppDelegate *GetInstance();
    static void ReleaseInstance();

    // 旧版初始化（使用 GLCore 窗口）
    bool Initialize(GLCore *window);
    // 新版初始化（自建 GLFW 窗口，暂未用到，保留接口）
    bool Initialize();

    void Release();
    void Run(); // 新版主循环

    void resize(int width, int height); // 旧版窗口大小变化
    void update();                      // 旧版每帧更新

    void OnMouseCallBack(GLFWwindow *window, int button, int action, int modify);
    void OnMouseCallBack(GLFWwindow *window, double x, double y);

    GLCore *GetWindow() { return _window; }
    LAppView *GetView() { return _view; }
    bool GetIsEnd() { return _isEnd; }
    void AppEnd() { _isEnd = true; }
    LAppTextureManager *GetTextureManager() { return _textureManager; }
    int GetWindowWidth() { return _windowWidth; }
    int GetWindowHeight() { return _windowHeight; }

    LAppDelegate();
    virtual ~LAppDelegate();

private:
    bool InitializeCubism();

    LAppAllocator_Common _cubismAllocator;
    Csm::CubismFramework::Option _cubismOption;

    GLCore *_window; // 旧版窗口
    LAppView *_view;
    LAppTextureManager *_textureManager;

    bool _captured;
    float _mouseX, _mouseY;
    bool _isEnd;
    int _windowWidth, _windowHeight;
};

class EventHandler
{
public:
    static void OnMouseCallBack(GLFWwindow *window, int button, int action, int modify)
    {
        LAppDelegate::GetInstance()->OnMouseCallBack(window, button, action, modify);
    }
    static void OnMouseCallBack(GLFWwindow *window, double x, double y)
    {
        LAppDelegate::GetInstance()->OnMouseCallBack(window, x, y);
    }
};