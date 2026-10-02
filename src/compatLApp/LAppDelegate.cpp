#include "LAppDelegate.hpp"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "LAppView.hpp"
#include "LAppPal.hpp"
#include "LAppDefine.hpp"
#include "LAppLive2DManager.hpp"
#include "LAppTextureManager.hpp"
#include "cubismcoreloader.hpp"

using namespace Csm;
using namespace std;
using namespace LAppDefine;

namespace
{
    LAppDelegate *s_instance = nullptr;
}

LAppDelegate *LAppDelegate::GetInstance()
{
    if (!s_instance)
        s_instance = new LAppDelegate();
    return s_instance;
}

void LAppDelegate::ReleaseInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

// ---------- 旧版初始化（使用 GLCore 窗口） ----------
bool LAppDelegate::Initialize(GLCore *window)
{
    if (DebugLogEnable)
        LAppPal::PrintLogLn("START");

    _window = window;
    if (_window == nullptr)
    {
        LAppPal::PrintLogLn("Can't create window.");
        return false;
    }
    _window->makeCurrent();
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
    {
        LAppPal::PrintLogLn("Can't initilize glew.");
        return false;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    _windowWidth = _window->width();
    _windowHeight = _window->height();

    InitializeCubism();
    _view->Initialize(_windowWidth, _windowHeight);

    return true;
}

// ---------- 新版初始化（备用） ----------
bool LAppDelegate::Initialize()
{
    // 这里仅作为接口保留，暂不实现 GLFW 窗口创建
    return false;
}

void LAppDelegate::Release()
{
    glfwTerminate();
    delete _textureManager;
    delete _view;
    LAppLive2DManager::ReleaseInstance();
    CubismFramework::Dispose();
}

void LAppDelegate::Run()
{
    while (true)
    {
        // 预留 GLFW 主循环，此处不实现
        break;
    }
}

// ---------- 窗口 resize ----------
void LAppDelegate::resize(int width, int height)
{
    if ((_windowWidth != width || _windowHeight != height) && width > 0 && height > 0)
    {
        _view->Initialize(width, height);
        _view->ResizeSprite();
        // 销毁旧的离屏渲染目标
        _view->DestroySpriteRenderTarget();
        // 更新所有模型的渲染目标尺寸
        LAppLive2DManager::GetInstance()->SetRenderTargetSize(width, height);
        _windowWidth = width;
        _windowHeight = height;
        glViewport(0, 0, width, height);
    }
    else
    {
        glViewport(0, 0, width, height);
    }
}

// ---------- 每帧更新（由 GLCore::paintGL 调用） ----------
void LAppDelegate::update()
{
    LAppPal::UpdateTime();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 透明背景
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);
    _view->Render();
}

// ---------- 构造 / 析构 ----------
LAppDelegate::LAppDelegate()
    : _cubismOption(),
      _window(nullptr),
      _captured(false),
      _mouseX(0.0f), _mouseY(0.0f),
      _isEnd(false),
      _windowWidth(0), _windowHeight(0)
{
    _view = new LAppView();
    _textureManager = new LAppTextureManager();
}

LAppDelegate::~LAppDelegate() {}

bool LAppDelegate::InitializeCubism()
{
    // load() 必须先于 StartUp()，否则 Core 存在时 csmSetLogFunction 无法解析。
    const bool coreReady = CubismCoreLoader::instance().load();
    if (!coreReady)
    {
        LAppPal::PrintLogLn("[Core] Live2D Cubism Core not installed, model disabled.");
    }

    // StartUp()/Initialize() 必须无条件执行，不能放进 coreReady 分支：
    // Framework 的 s_allocator 只在 StartUp() 里赋值，缺了它 LAppPal::LoadFileAsBytes
    // 的 CSM_MALLOC、csmString 等全部空指针崩溃（LAppView::Initialize 建 shader 即触发）。
    // Core DLL 缺失只影响 moc/渲染解析，已由 LAppLive2DManager 的两处守卫拦截。
    _cubismOption.LogFunction = LAppPal::PrintMessage;
    _cubismOption.LoggingLevel = LAppDefine::CubismLoggingLevel;
    _cubismOption.LoadFileFunction = LAppPal::LoadFileAsBytes;
    _cubismOption.ReleaseBytesFunction = LAppPal::ReleaseBytes;
    Csm::CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);
    CubismFramework::Initialize();

    LAppLive2DManager::GetInstance();
    CubismMatrix44 projection;
    LAppPal::UpdateTime();
    return coreReady;
}

// ---------- 鼠标回调 ----------
void LAppDelegate::OnMouseCallBack(GLFWwindow *window, int button, int action, int modify)
{
    if (!_view)
        return;
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_PRESS)
    {
        _captured = true;
        _view->OnTouchesBegan(_mouseX, _mouseY);
    }
    else if (action == GLFW_RELEASE)
    {
        if (_captured)
        {
            _captured = false;
            _view->OnTouchesEnded(_mouseX, _mouseY);
        }
    }
}

void LAppDelegate::OnMouseCallBack(GLFWwindow *window, double x, double y)
{
    _mouseX = static_cast<float>(x);
    _mouseY = static_cast<float>(y);
    if (!_captured || !_view)
        return;
    _view->OnTouchesMoved(_mouseX, _mouseY);
}