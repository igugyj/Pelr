#include "LAppView.hpp"
#include <math.h>
#include <string>
#include "LAppPal.hpp"
#include "LAppDelegate.hpp"
#include "LAppLive2DManager.hpp"
#include "LAppTextureManager.hpp"
#include "LAppDefine.hpp"
#include "TouchManager_Common.hpp"
#include "LAppSprite.hpp"
#include "LAppSpriteShader.hpp"
#include "LAppModel.hpp"
#include "GLCore.h"

using namespace std;
using namespace LAppDefine;

LAppView::LAppView()
    : LAppView_Common(),
      _back(nullptr), _gear(nullptr), _power(nullptr),
      _renderSprite(nullptr),
      _renderTarget(SelectTarget_None),
      _spriteShader(nullptr)
{
    _clearColor[0] = 1.0f;
    _clearColor[1] = 1.0f;
    _clearColor[2] = 1.0f;
    _clearColor[3] = 0.0f;
    _touchManager = new TouchManager_Common();
}

LAppView::~LAppView()
{
    _renderBuffer.DestroyRenderTarget();
    delete _renderSprite;
    delete _spriteShader;
    delete _touchManager;
    delete _back;
    delete _gear;
    delete _power;
}

void LAppView::Initialize(int width, int height)
{
    LAppView_Common::Initialize(width, height);
    if (!_spriteShader)
        _spriteShader = new LAppSpriteShader();
    if (!_back)
        InitializeSprite();
}

void LAppView::Render()
{
    GLCore *window = LAppDelegate::GetInstance()->GetWindow();
    int maxWidth = window->width();
    int maxHeight = window->height();
    // 清除帧缓冲，使背景透明（用于像素级碰撞检测等）
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // _back->SetWindowSize(maxWidth, maxHeight);
    // _gear->SetWindowSize(maxWidth, maxHeight);
    // _power->SetWindowSize(maxWidth, maxHeight);

    // _back->Render();
    // _gear->Render();
    // _power->Render();

    LAppLive2DManager *live2D = LAppLive2DManager::GetInstance();
    live2D->SetViewMatrix(_viewMatrix);
    live2D->OnUpdate();

    if (_renderTarget == SelectTarget_ModelFrameBuffer && _renderSprite)
    {
        const GLfloat uv[] = {1, 1, 0, 1, 0, 0, 1, 0};
        for (csmUint32 i = 0; i < live2D->GetModelNum(); i++)
        {
            LAppModel *model = live2D->GetModel(i);
            float alpha = (i == 0) ? 1.0f : model->GetOpacity();
            _renderSprite->SetColor(1.0f * alpha, 1.0f * alpha, 1.0f * alpha, alpha);
            if (model)
            {
                _renderSprite->SetWindowSize(maxWidth, maxHeight);
                _renderSprite->RenderImmidiate(model->GetRenderBuffer().GetColorBuffer(), uv);
            }
        }
    }
}

void LAppView::InitializeSprite()
{
    GLuint programId = _spriteShader->GetShaderId();
    GLCore *window = LAppDelegate::GetInstance()->GetWindow();
    int width = window->width();
    int height = window->height();

    LAppTextureManager *texMgr = LAppDelegate::GetInstance()->GetTextureManager();
    const string resPath = ResourcesPath;

    // _back
    /*
    {
        string imgName = BackImageName;
        auto texInfo = texMgr->CreateTextureFromPngFile(resPath + imgName);
        float fHeight = static_cast<float>(height * 0.95f);
        float ratio = fHeight / static_cast<float>(texInfo->height);
        float fWidth = static_cast<float>(texInfo->width) * ratio;
        _back = new LAppSprite(width * 0.5f, height * 0.5f, fWidth, fHeight, texInfo->id, programId);
    }

    // _gear
    {
        string imgName = GearImageName;
        auto texInfo = texMgr->CreateTextureFromPngFile(resPath + imgName);
        float x = width - texInfo->width * 0.5f;
        float y = height - texInfo->height * 0.5f;
        _gear = new LAppSprite(x, y, static_cast<float>(texInfo->width),
                               static_cast<float>(texInfo->height), texInfo->id, programId);
    }
    // _power
    {
        string imgName = PowerImageName;
        auto texInfo = texMgr->CreateTextureFromPngFile(resPath + imgName);
        float x = width - texInfo->width * 0.5f;
        float y = texInfo->height * 0.5f;
        _power = new LAppSprite(x, y, static_cast<float>(texInfo->width),
                                static_cast<float>(texInfo->height), texInfo->id, programId);
    }
    // _renderSprite
    _renderSprite = new LAppSprite(width * 0.5f, height * 0.5f,
                                   static_cast<float>(width), static_cast<float>(height), 0, programId);
*/
}

// ---------- 触摸处理 ----------
void LAppView::OnTouchesBegan(float px, float py) const
{
    _touchManager->TouchesBegan(px, py);
}
void LAppView::OnTouchesMoved(float px, float py) const
{
    float vx = TransformViewX(_touchManager->GetX());
    float vy = TransformViewY(_touchManager->GetY());
    _touchManager->TouchesMoved(px, py);
    LAppLive2DManager::GetInstance()->OnDrag(vx, vy);
}
void LAppView::OnTouchesEnded(float px, float py) const
{
    LAppLive2DManager *mgr = LAppLive2DManager::GetInstance();
    mgr->OnDrag(0.0f, 0.0f);
    float x = _deviceToScreen->TransformX(px);
    float y = _deviceToScreen->TransformY(py);
    mgr->OnTap(x, y);
    /*
    if (_gear->IsHit(px, py))
        mgr->NextScene();
    if (_power->IsHit(px, py))
        LAppDelegate::GetInstance()->AppEnd();
        */
}

// ---------- 离屏渲染相关 ----------
void LAppView::PreModelDraw(LAppModel &refModel)
{
    Csm::Rendering::CubismRenderTarget_OpenGLES2 *target = nullptr;
    if (_renderTarget != SelectTarget_None)
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        target = (_renderTarget == SelectTarget_ViewFrameBuffer) ? &_renderBuffer : &refModel.GetRenderBuffer();
        if (!target->IsValid())
        {
            GLCore *w = LAppDelegate::GetInstance()->GetWindow();
            int ww = w->width(), wh = w->height();
            if (ww > 0 && wh > 0)
                target->CreateRenderTarget(ww, wh);
        }
        target->BeginDraw();
        target->Clear(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);
    }
}

void LAppView::PostModelDraw(LAppModel &refModel)
{
    Csm::Rendering::CubismRenderTarget_OpenGLES2 *target = nullptr;
    if (_renderTarget != SelectTarget_None)
    {
        target = (_renderTarget == SelectTarget_ViewFrameBuffer) ? &_renderBuffer : &refModel.GetRenderBuffer();
        target->EndDraw();
        if (_renderTarget == SelectTarget_ViewFrameBuffer && _renderSprite)
        {
            const GLfloat uv[] = {1, 1, 0, 1, 0, 0, 1, 0};
            _renderSprite->SetColor(1.0f * GetSpriteAlpha(0), 1.0f * GetSpriteAlpha(0),
                                    1.0f * GetSpriteAlpha(0), GetSpriteAlpha(0));
            GLCore *w = LAppDelegate::GetInstance()->GetWindow();
            _renderSprite->SetWindowSize(w->width(), w->height());
            _renderSprite->RenderImmidiate(target->GetColorBuffer(), uv);
        }
    }
}

void LAppView::SwitchRenderingTarget(SelectTarget type) { _renderTarget = type; }
void LAppView::SetRenderTargetClearColor(float r, float g, float b)
{
    _clearColor[0] = r;
    _clearColor[1] = g;
    _clearColor[2] = b;
}
float LAppView::GetSpriteAlpha(int assign) const
{
    float alpha = 0.4f + assign * 0.5f;
    if (alpha > 1.0f)
        alpha = 1.0f;
    if (alpha < 0.1f)
        alpha = 0.1f;
    return alpha;
}

void LAppView::ResizeSprite()
{
    LAppTextureManager *texMgr = LAppDelegate::GetInstance()->GetTextureManager();
    if (!texMgr)
        return;
    GLCore *window = LAppDelegate::GetInstance()->GetWindow();
    int w = window->width(), h = window->height();
    /*
        // 背景
        if (_back)
        {
            auto info = texMgr->GetTextureInfoById(_back->GetTextureId());
            if (info)
            {
                float fh = h * 0.95f;
                float ratio = fh / info->height;
                _back->ResetRect(w * 0.5f, h * 0.5f, info->width * ratio, fh);
            }
        }
        // 齿轮
        if (_gear)
        {
            auto info = texMgr->GetTextureInfoById(_gear->GetTextureId());
            if (info)
                _gear->ResetRect(w - info->width * 0.5f, h - info->height * 0.5f, info->width, info->height);
        }
        // 电源
        if (_power)
        {
            auto info = texMgr->GetTextureInfoById(_power->GetTextureId());
            if (info)
                _power->ResetRect(w - info->width * 0.5f, info->height * 0.5f, info->width, info->height);
        }
        if (_renderSprite)
            _renderSprite->ResetRect(w * 0.5f, h * 0.5f, static_cast<float>(w), static_cast<float>(h));
    */
}

void LAppView::DestroySpriteRenderTarget()
{
    LAppLive2DManager *mgr = LAppLive2DManager::GetInstance();
    if (_renderTarget == SelectTarget_ViewFrameBuffer)
    {
        _renderBuffer.DestroyRenderTarget();
    }
    else if (_renderTarget == SelectTarget_ModelFrameBuffer)
    {
        for (csmUint32 i = 0; i < mgr->GetModelNum(); i++)
        {
            LAppModel *model = mgr->GetModel(i);
            if (model)
                model->GetRenderBuffer().DestroyRenderTarget();
        }
    }
}