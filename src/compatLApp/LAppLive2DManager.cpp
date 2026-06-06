#include "LAppLive2DManager.hpp"
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Rendering/CubismRenderer.hpp>
#include "LAppPal.hpp"
#include "LAppDefine.hpp"
#include "LAppDelegate.hpp"
#include "LAppModel.hpp"
#include "LAppView.hpp"
#include "data.hpp" // DataManager
#include <QFileInfo>
#include <QDebug>

using namespace Csm;
using namespace LAppDefine;
using namespace std;

Csm::csmFloat32 LAppLive2DManager::_dragStrength = 1.0f;

namespace
{
    LAppLive2DManager *s_instance = nullptr;

    void BeganMotion(ACubismMotion *self)
    {
        LAppPal::PrintLogLn("Motion Began: %x", self);
    }
    void FinishedMotion(ACubismMotion *self)
    {
        LAppPal::PrintLogLn("Motion Finished: %x", self);
    }
    int CompareCsmString(const void *a, const void *b)
    {
        return strcmp(reinterpret_cast<const csmString *>(a)->GetRawString(),
                      reinterpret_cast<const csmString *>(b)->GetRawString());
    }
}

LAppLive2DManager *LAppLive2DManager::GetInstance()
{
    if (!s_instance)
        s_instance = new LAppLive2DManager();
    return s_instance;
}

void LAppLive2DManager::ReleaseInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

LAppLive2DManager::LAppLive2DManager()
    : _viewMatrix(nullptr), _sceneIndex(0)
{
    _viewMatrix = new CubismMatrix44();
    LoadModelFromConfig(); // 启动时从配置加载模型
}

LAppLive2DManager::~LAppLive2DManager()
{
    ReleaseAllModel();
    delete _viewMatrix;
}

// ---------- 从文件路径加载模型 ----------
void LAppLive2DManager::LoadModelFromPath(const std::string &modelPath, const std::string &fileName)
{
    ReleaseAllModel();
    _models.PushBack(new LAppModel());
    _models[0]->LoadAssets(modelPath.c_str(), fileName.c_str());
}

// ---------- 从配置文件加载模型 ----------
void LAppLive2DManager::LoadModelFromConfig()
{
    QString modelPath = DataManager::instance().getBasicData().model_path;
    if (modelPath.isEmpty())
    {
        qCritical() << "[LApp] Model path is empty, using default model";
        modelPath = "Resources/Hiyori/Hiyori.model3.json";
    }
    QFileInfo fi(modelPath);
    if (!fi.exists())
    {
        qCritical() << "[LApp] Model file not exists:" << modelPath;
        modelPath = "Resources/Hiyori/Hiyori.model3.json";
        fi.setFile(modelPath);
    }
    QString dir = fi.absolutePath();
    QString fName = fi.fileName();
    qDebug() << "[LApp] Loading model from config:" << dir << fName;
    LoadModelFromPath(dir.toStdString() + "/", fName.toStdString());
}

// ---------- SetUpModel（扫描 Resources 目录） ----------
void LAppLive2DManager::SetUpModel()
{
    csmString crawlPath(ResourcesPath);
    crawlPath += "*.*";
    wchar_t wideStr[MAX_PATH];
    csmChar name[MAX_PATH];
    LAppPal::ConvertMultiByteToWide(crawlPath.GetRawString(), wideStr, MAX_PATH);
    struct _wfinddata_t fdata;
    intptr_t fh = _wfindfirst(wideStr, &fdata);
    if (fh == -1)
        return;
    _modelDir.Clear();
    while (_wfindnext(fh, &fdata) == 0)
    {
        if ((fdata.attrib & _A_SUBDIR) && wcscmp(fdata.name, L"..") != 0)
        {
            LAppPal::ConvertWideToMultiByte(fdata.name, name, MAX_PATH);
            csmString model3jsonPath(ResourcesPath);
            model3jsonPath += name;
            model3jsonPath.Append(1, '/');
            model3jsonPath += name;
            model3jsonPath += ".model3.json";
            LAppPal::ConvertMultiByteToWide(model3jsonPath.GetRawString(), wideStr, MAX_PATH);
            struct _wfinddata_t fdata2;
            if (_wfindfirst(wideStr, &fdata2) != -1)
            {
                _modelDir.PushBack(csmString(name));
            }
        }
    }
    qsort(_modelDir.GetPtr(), _modelDir.GetSize(), sizeof(csmString), CompareCsmString);
}

csmVector<csmString> LAppLive2DManager::GetModelDir() const { return _modelDir; }
csmInt32 LAppLive2DManager::GetModelDirSize() const { return _modelDir.GetSize(); }

LAppModel *LAppLive2DManager::GetModel(csmUint32 no) const
{
    return (no < _models.GetSize()) ? _models[no] : nullptr;
}

void LAppLive2DManager::ReleaseAllModel()
{
    for (csmUint32 i = 0; i < _models.GetSize(); ++i)
        delete _models[i];
    _models.Clear();
}

void LAppLive2DManager::SetRenderTargetSize(csmUint32 width, csmUint32 height)
{
    for (csmUint32 i = 0; i < _models.GetSize(); ++i)
    {
        LAppModel *model = GetModel(i);
        if (model)
            model->SetRenderTargetSize(width, height);
    }
}

void LAppLive2DManager::SetDragStrength(csmFloat32 strength)
{
    _dragStrength = strength;
    qDebug() << "[LApp] Drag strength: " << _dragStrength;
}

void LAppLive2DManager::OnDrag(csmFloat32 x, csmFloat32 y) const
{
    x *= _dragStrength;
    y *= _dragStrength;
    for (csmUint32 i = 0; i < _models.GetSize(); ++i)
    {
        LAppModel *model = GetModel(i);
        if (model)
            model->SetDragging(x, y);
    }
}

void LAppLive2DManager::OnTap(csmFloat32 x, csmFloat32 y)
{
    if (DebugLogEnable)
        LAppPal::PrintLogLn("[APP]tap point: {x:%.2f y:%.2f}", x, y);
    for (csmUint32 i = 0; i < _models.GetSize(); ++i)
    {
        LAppModel *model = _models[i];
        if (!model)
            continue;
        if (model->HitTest(HitAreaNameHead, x, y))
        {
            if (DebugLogEnable)
                LAppPal::PrintLogLn("[APP]hit area: [%s]", HitAreaNameHead);
            model->SetRandomExpression();
        }
        else if (model->HitTest(HitAreaNameBody, x, y))
        {
            if (DebugLogEnable)
                LAppPal::PrintLogLn("[APP]hit area: [%s]", HitAreaNameBody);
            model->StartRandomMotion(MotionGroupTapBody, PriorityNormal, FinishedMotion, BeganMotion);
        }
    }
}

void LAppLive2DManager::OnUpdate() const
{
    GLCore *window = LAppDelegate::GetInstance()->GetWindow();
    int width = window->width();
    int height = window->height();

    for (csmUint32 i = 0; i < _models.GetSize(); ++i)
    {
        LAppModel *model = GetModel(i);
        if (!model || !model->GetModel())
            continue;

        CubismMatrix44 projection;
        if (model->GetModel()->GetCanvasWidth() > 1.0f && width < height)
        {
            model->GetModelMatrix()->SetWidth(2.0f);
            projection.Scale(1.0f, static_cast<float>(width) / height);
        }
        else
        {
            projection.Scale(static_cast<float>(height) / width, 1.0f);
        }
        if (_viewMatrix)
            projection.MultiplyByMatrix(_viewMatrix);

        LAppDelegate::GetInstance()->GetView()->PreModelDraw(*model);
        model->Update();
        model->Draw(projection);
        LAppDelegate::GetInstance()->GetView()->PostModelDraw(*model);
    }
}

void LAppLive2DManager::NextScene()
{
    csmInt32 no = (_sceneIndex + 1) % GetModelDirSize();
    ChangeScene(no);
}

void LAppLive2DManager::ChangeScene(Csm::csmInt32 index)
{
    _sceneIndex = index;
    if (DebugLogEnable)
        LAppPal::PrintLogLn("[APP]model index: %d", _sceneIndex);
    const csmString &model = _modelDir[index];
    csmString modelPath(ResourcesPath);
    modelPath += model;
    modelPath.Append(1, '/');
    csmString modelJsonName(model);
    modelJsonName += ".model3.json";
    ReleaseAllModel();
    _models.PushBack(new LAppModel());
    _models[0]->LoadAssets(modelPath.GetRawString(), modelJsonName.GetRawString());
}

csmUint32 LAppLive2DManager::GetModelNum() const { return _models.GetSize(); }

void LAppLive2DManager::SetViewMatrix(CubismMatrix44 *m)
{
    for (int i = 0; i < 16; ++i)
        _viewMatrix->GetArray()[i] = m->GetArray()[i];
}

// ---------- TTS 口形同步代理 ----------
void LAppLive2DManager::StartLipSync(const Csm::csmString &wavFilePath)
{
    StartLipSync(0, wavFilePath);
}

void LAppLive2DManager::StartLipSync(Csm::csmUint32 modelIndex, const Csm::csmString &wavFilePath)
{
    LAppModel *model = GetModel(modelIndex);
    if (model)
        model->StartLipSync(wavFilePath);
    else
        qWarning() << "[LAppLive2DManager] No model at index" << modelIndex;
}