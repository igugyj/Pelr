#include "LAppModel.hpp"
#include <fstream>
#include <CubismModelSettingJson.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <CubismDefaultParameterId.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>
#include <Utils/CubismString.hpp>
#include <Id/CubismIdManager.hpp>
#include <Motion/CubismMotionQueueEntry.hpp>
#include "LAppDefine.hpp"
#include "LAppPal.hpp"
#include "LAppTextureManager.hpp"
#include "LAppDelegate.hpp"
#include "Motion/CubismBreathUpdater.hpp"
#include "Motion/CubismLookUpdater.hpp"
#include "Motion/CubismExpressionUpdater.hpp"
#include "Motion/CubismEyeBlinkUpdater.hpp"
#include "Motion/CubismLipSyncUpdater.hpp"
#include "Motion/CubismPhysicsUpdater.hpp"
#include "Motion/CubismPoseUpdater.hpp"
#include "ExtraMotionManager.h"
#include <QDebug>
#include <QRandomGenerator>

using namespace Live2D::Cubism::Framework;
using namespace Live2D::Cubism::Framework::DefaultParameterId;
using namespace LAppDefine;

LAppModel::LAppModel()
    : LAppModel_Common(), _modelSetting(nullptr), _userTimeSeconds(0.0f), _motionUpdated(false), _extraFileManager(new ExtraFileManager())
{
    if (MocConsistencyValidationEnable)
        _mocConsistency = true;
    if (MotionConsistencyValidationEnable)
        _motionConsistency = true;
    if (DebugLogEnable)
        _debugMode = true;

    _idParamAngleX = CubismFramework::GetIdManager()->GetId(ParamAngleX);
    _idParamAngleY = CubismFramework::GetIdManager()->GetId(ParamAngleY);
    _idParamAngleZ = CubismFramework::GetIdManager()->GetId(ParamAngleZ);
    _idParamBodyAngleX = CubismFramework::GetIdManager()->GetId(ParamBodyAngleX);
    _idParamEyeBallX = CubismFramework::GetIdManager()->GetId(ParamEyeBallX);
    _idParamEyeBallY = CubismFramework::GetIdManager()->GetId(ParamEyeBallY);
}

LAppModel::~LAppModel()
{
    _renderBuffer.DestroyRenderTarget();
    ReleaseMotions();
    ReleaseExpressions();

    // 释放额外动作和表情
    for (auto it = extraMotions.Begin(); it != extraMotions.End(); ++it)
        ACubismMotion::Delete(it->Second);
    extraMotions.Clear();
    for (auto it = extraExpressions.Begin(); it != extraExpressions.End(); ++it)
        ACubismMotion::Delete(it->Second);
    extraExpressions.Clear();

    for (csmInt32 i = 0; i < _modelSetting->GetMotionGroupCount(); i++)
    {
        const csmChar *group = _modelSetting->GetMotionGroupName(i);
        ReleaseMotionGroup(group);
    }
    delete _modelSetting;
    delete _extraFileManager;
}

// ---------- 加载资源 ----------
void LAppModel::LoadAssets(const csmChar *dir, const csmChar *fileName)
{
    _modelHomeDir = dir;
    if (_debugMode)
    {
        LAppPal::PrintLogLn("[APP]load model setting: %s", fileName);
        qDebug() << "[LApp] Load model setting:" << fileName;
    }
    csmSizeInt size;
    csmString path = csmString(dir) + fileName;
    csmByte *buffer = CreateBuffer(path.GetRawString(), &size);
    ICubismModelSetting *setting = new CubismModelSettingJson(buffer, size);
    DeleteBuffer(buffer, path.GetRawString());
    SetupModel(setting);
    if (_model == nullptr)
    {
        LAppPal::PrintLogLn("Failed to LoadAssets().");
        return;
    }
    CreateRenderer(LAppDelegate::GetInstance()->GetWindowWidth(),
                   LAppDelegate::GetInstance()->GetWindowHeight());
    SetupTextures();
}

// ---------- 设置模型（使用新版完整的 SetupModel 流程，末尾追加 LoadExtraFiles） ----------
void LAppModel::SetupModel(ICubismModelSetting *setting)
{
    _updating = true;
    _initialized = false;
    _modelSetting = setting;

    csmByte *buffer;
    csmSizeInt size;

    // Model
    if (strcmp(setting->GetModelFileName(), "") != 0)
    {
        csmString path = setting->GetModelFileName();
        path = _modelHomeDir + path;
        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadModel(buffer, size, _mocConsistency);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // Expressions
    if (setting->GetExpressionCount() > 0)
    {
        const csmInt32 count = setting->GetExpressionCount();
        for (csmInt32 i = 0; i < count; i++)
        {
            csmString name = setting->GetExpressionName(i);
            csmString path = setting->GetExpressionFileName(i);
            path = _modelHomeDir + path;
            buffer = CreateBuffer(path.GetRawString(), &size);
            ACubismMotion *motion = LoadExpression(buffer, size, name.GetRawString());
            if (motion)
            {
                if (_expressions[name] != nullptr)
                    ACubismMotion::Delete(_expressions[name]);
                _expressions[name] = motion;
            }
            DeleteBuffer(buffer, path.GetRawString());
        }
        // CubismExpressionUpdater *expr = CSM_NEW CubismExpressionUpdater(*_expressionManager);
        // _updateScheduler.AddUpdatableList(expr);
    }

    // Physics
    if (strcmp(setting->GetPhysicsFileName(), "") != 0)
    {
        csmString path = setting->GetPhysicsFileName();
        path = _modelHomeDir + path;
        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadPhysics(buffer, size);
        if (_physics)
        {
            CubismPhysicsUpdater *phys = CSM_NEW CubismPhysicsUpdater(*_physics);
            _updateScheduler.AddUpdatableList(phys);
        }
        DeleteBuffer(buffer, path.GetRawString());
    }

    // Pose
    if (strcmp(setting->GetPoseFileName(), "") != 0)
    {
        csmString path = setting->GetPoseFileName();
        path = _modelHomeDir + path;
        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadPose(buffer, size);
        if (_pose)
        {
            CubismPoseUpdater *pose = CSM_NEW CubismPoseUpdater(*_pose);
            _updateScheduler.AddUpdatableList(pose);
        }
        DeleteBuffer(buffer, path.GetRawString());
    }

    // EyeBlink
    if (setting->GetEyeBlinkParameterCount() > 0)
    {
        _eyeBlink = CubismEyeBlink::Create(setting);
        CubismEyeBlinkUpdater *blink = CSM_NEW CubismEyeBlinkUpdater(_motionUpdated, *_eyeBlink);
        _updateScheduler.AddUpdatableList(blink);
    }

    // Breath
    {
        _breath = CubismBreath::Create();
        csmVector<CubismBreath::BreathParameterData> params;
        params.PushBack(CubismBreath::BreathParameterData(_idParamAngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
        params.PushBack(CubismBreath::BreathParameterData(_idParamAngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
        params.PushBack(CubismBreath::BreathParameterData(_idParamAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
        params.PushBack(CubismBreath::BreathParameterData(_idParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
        params.PushBack(CubismBreath::BreathParameterData(
            CubismFramework::GetIdManager()->GetId(ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));
        _breath->SetParameters(params);
        CubismBreathUpdater *breath = CSM_NEW CubismBreathUpdater(*_breath);
        _updateScheduler.AddUpdatableList(breath);
    }

    // UserData
    if (strcmp(setting->GetUserDataFile(), "") != 0)
    {
        csmString path = setting->GetUserDataFile();
        path = _modelHomeDir + path;
        buffer = CreateBuffer(path.GetRawString(), &size);
        LoadUserData(buffer, size);
        DeleteBuffer(buffer, path.GetRawString());
    }

    // EyeBlinkIds
    {
        csmInt32 count = setting->GetEyeBlinkParameterCount();
        for (csmInt32 i = 0; i < count; i++)
            _eyeBlinkIds.PushBack(setting->GetEyeBlinkParameterId(i));
    }

    // LipSyncIds
    {
        csmInt32 count = setting->GetLipSyncParameterCount();
        for (csmInt32 i = 0; i < count; i++)
            _lipSyncIds.PushBack(setting->GetLipSyncParameterId(i));
        CubismLipSyncUpdater *lipSync = CSM_NEW CubismLipSyncUpdater(_lipSyncIds, _wavFileHandler);
        _updateScheduler.AddUpdatableList(lipSync);
    }

    // Look
    {
        _look = CubismLook::Create();
        csmVector<CubismLook::LookParameterData> lookParams;
        lookParams.PushBack(CubismLook::LookParameterData(_idParamAngleX, 30.0f));
        lookParams.PushBack(CubismLook::LookParameterData(_idParamAngleY, 0.0f, 30.0f));
        lookParams.PushBack(CubismLook::LookParameterData(_idParamAngleZ, 0.0f, 0.0f, -30.0f));
        lookParams.PushBack(CubismLook::LookParameterData(_idParamBodyAngleX, 10.0f));
        lookParams.PushBack(CubismLook::LookParameterData(_idParamEyeBallX, 1.0f));
        lookParams.PushBack(CubismLook::LookParameterData(_idParamEyeBallY, 0.0f, 1.0f));
        _look->SetParameters(lookParams);
        CubismLookUpdater *look = CSM_NEW CubismLookUpdater(*_look, *_dragManager);
        _updateScheduler.AddUpdatableList(look);
    }

    _updateScheduler.SortUpdatableList();

    // Layout
    csmMap<csmString, csmFloat32> layout;
    _modelSetting->GetLayoutMap(layout);
    _modelMatrix->SetupFromLayout(layout);

    _model->SaveParameters();

    for (csmInt32 i = 0; i < _modelSetting->GetMotionGroupCount(); i++)
        PreloadMotionGroup(_modelSetting->GetMotionGroupName(i));

    _motionManager->StopAllMotions();

    _updating = false;
    _initialized = true;

    // 加载额外文件（动作、表情）
    LoadExtraFiles();
}

// ---------- 预加载动作组 ----------
void LAppModel::PreloadMotionGroup(const csmChar *group)
{
    const csmInt32 count = _modelSetting->GetMotionCount(group);
    for (csmInt32 i = 0; i < count; i++)
    {
        csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, i);
        csmString path = _modelSetting->GetMotionFileName(group, i);
        path = _modelHomeDir + path;

        if (_debugMode)
            LAppPal::PrintLogLn("[APP]load motion: %s => [%s_%d] ", path.GetRawString(), group, i);

        csmByte *buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        CubismMotion *tmpMotion = static_cast<CubismMotion *>(LoadMotion(buffer, size, name.GetRawString(), NULL, NULL, _modelSetting, group, i, _motionConsistency));

        if (tmpMotion)
        {
            tmpMotion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);
            if (_motions[name] != NULL)
                ACubismMotion::Delete(_motions[name]);
            _motions[name] = tmpMotion;
        }
        DeleteBuffer(buffer, path.GetRawString());
    }
}

// ---------- 释放动作组 ----------
void LAppModel::ReleaseMotionGroup(const csmChar *group) const
{
    const csmInt32 count = _modelSetting->GetMotionCount(group);
    for (csmInt32 i = 0; i < count; i++)
    {
        csmString voice = _modelSetting->GetMotionSoundFileName(group, i);
        if (strcmp(voice.GetRawString(), "") != 0)
        {
            csmString path = voice;
            path = _modelHomeDir + path;
        }
    }
}

// ---------- 释放所有动作 ----------
void LAppModel::ReleaseMotions()
{
    for (csmMap<csmString, ACubismMotion *>::const_iterator iter = _motions.Begin(); iter != _motions.End(); ++iter)
        ACubismMotion::Delete(iter->Second);
    _motions.Clear();
}

// ---------- 释放所有表情 ----------
void LAppModel::ReleaseExpressions()
{
    for (csmMap<csmString, ACubismMotion *>::const_iterator iter = _expressions.Begin(); iter != _expressions.End(); ++iter)
        ACubismMotion::Delete(iter->Second);
    _expressions.Clear();
}

// ---------- 更新 ----------
void LAppModel::Update()
{
    const csmFloat32 deltaTimeSeconds = LAppPal::GetDeltaTime();
    _userTimeSeconds += deltaTimeSeconds;

    // 仅在 deltaTime 异常时输出警告
    if (deltaTimeSeconds <= 0.0f)
    {
        static bool warnedDeltaZero = false;
        if (!warnedDeltaZero)
        {
            LAppPal::PrintLogLn("[Update] WARNING: deltaTime is zero or negative (%.4f). Animation will freeze.", deltaTimeSeconds);
            warnedDeltaZero = true;
        }
    }

    _motionUpdated = false;

    _model->LoadParameters();

    csmBool isFinished = _motionManager->IsFinished();
    if (isFinished)
    {
        csmInt32 idleCount = _modelSetting->GetMotionCount(MotionGroupIdle);
        if (idleCount == 0)
        {
            // Idle 动作缺失属于严重问题，每次 Finished 都报告一次（但不会每帧，因为 StartRandomMotion 后就不再 Finished）
            // LAppPal::PrintLogLn("[Update] WARNING: No Idle motions found. Model will stay static.");
        }
        StartRandomMotion(MotionGroupIdle, PriorityIdle);
    }
    else
    {
        _motionUpdated = _motionManager->UpdateMotion(_model, deltaTimeSeconds);
    }

    _model->SaveParameters();

    // 表情管理器空指针检查，只警告一次
    if (_expressionManager == NULL)
    {
        static bool warnedExpressionNull = false;
        if (!warnedExpressionNull)
        {
            LAppPal::PrintLogLn("[Update] WARNING: expressionManager is NULL. Expressions won't work.");
            warnedExpressionNull = true;
        }
    }
    else
    {
        _expressionManager->UpdateMotion(_model, deltaTimeSeconds);
    }

    _opacity = _model->GetModelOpacity();

    _updateScheduler.OnLateUpdate(_model, deltaTimeSeconds);

    _model->Update();
}
// ---------- 绘制 ----------
void LAppModel::Draw(CubismMatrix44 &matrix)
{
    if (_model == NULL)
        return;
    matrix.MultiplyByMatrix(_modelMatrix);
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->SetMvpMatrix(&matrix);
    DoDraw();
}

// ---------- 开始动作 ----------
CubismMotionQueueEntryHandle LAppModel::StartMotion(const csmChar *group, csmInt32 no, csmInt32 priority,
                                                    ACubismMotion::FinishedMotionCallback onFinished, ACubismMotion::BeganMotionCallback onBegan)
{
    if (priority == PriorityForce)
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        if (_debugMode)
            LAppPal::PrintLogLn("[APP]can't start motion.");
        return InvalidMotionQueueEntryHandleValue;
    }

    const csmString fileName = _modelSetting->GetMotionFileName(group, no);
    csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, no);
    CubismMotion *motion = static_cast<CubismMotion *>(_motions[name.GetRawString()]);
    csmBool autoDelete = false;

    if (motion == NULL)
    {
        csmString path = fileName;
        path = _modelHomeDir + path;
        csmByte *buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        motion = static_cast<CubismMotion *>(LoadMotion(buffer, size, NULL, onFinished, onBegan, _modelSetting, group, no, _motionConsistency));
        if (motion)
        {
            motion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);
            autoDelete = true;
        }
        else
        {
            CubismLogError("Can't start motion %s .", path.GetRawString());
            _motionManager->SetReservePriority(PriorityNone);
            DeleteBuffer(buffer, path.GetRawString());
            return InvalidMotionQueueEntryHandleValue;
        }
        DeleteBuffer(buffer, path.GetRawString());
    }
    else
    {
        motion->SetBeganMotionHandler(onBegan);
        motion->SetFinishedMotionHandler(onFinished);
    }

    csmString voice = _modelSetting->GetMotionSoundFileName(group, no);
    if (strcmp(voice.GetRawString(), "") != 0)
    {
        csmString path = voice;
        path = _modelHomeDir + path;
        _wavFileHandler.Start(path);
    }

    if (_debugMode)
        LAppPal::PrintLogLn("[APP]start motion: [%s_%d]", group, no);
    return _motionManager->StartMotionPriority(motion, autoDelete, priority);
}

// ---------- 随机动作 ----------
CubismMotionQueueEntryHandle LAppModel::StartRandomMotion(const csmChar *group, csmInt32 priority,
                                                          ACubismMotion::FinishedMotionCallback onFinished, ACubismMotion::BeganMotionCallback onBegan)
{
    if (_modelSetting->GetMotionCount(group) == 0)
        return InvalidMotionQueueEntryHandleValue;
    csmInt32 no = QRandomGenerator::global()->bounded(_modelSetting->GetMotionCount(group));
    return StartMotion(group, no, priority, onFinished, onBegan);
}

// ---------- 内部绘制 ----------
void LAppModel::DoDraw()
{
    if (_model == NULL)
        return;
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->DrawModel();
}

// ---------- 点击测试 ----------
csmBool LAppModel::HitTest(const csmChar *hitAreaName, csmFloat32 x, csmFloat32 y)
{
    if (_opacity < 1)
        return false;
    const csmInt32 count = _modelSetting->GetHitAreasCount();
    for (csmInt32 i = 0; i < count; i++)
    {
        if (strcmp(_modelSetting->GetHitAreaName(i), hitAreaName) == 0)
        {
            const CubismIdHandle drawID = _modelSetting->GetHitAreaId(i);
            return IsHit(drawID, x, y);
        }
    }
    return false;
}

// ---------- 设置表情 ----------
void LAppModel::SetExpression(const csmChar *expressionID)
{
    ACubismMotion *motion = _expressions[expressionID];
    if (_debugMode)
        LAppPal::PrintLogLn("[APP]expression: [%s]", expressionID);
    if (motion != NULL)
        _expressionManager->StartMotion(motion, false);
    else if (_debugMode)
        LAppPal::PrintLogLn("[APP]expression[%s] is null ", expressionID);
}

// ---------- 随机表情 ----------
void LAppModel::SetRandomExpression()
{
    if (_expressions.GetSize() == 0)
        return;
    csmInt32 no = QRandomGenerator::global()->bounded(_expressions.GetSize());
    csmMap<csmString, ACubismMotion *>::const_iterator iter;
    csmInt32 i = 0;
    for (iter = _expressions.Begin(); iter != _expressions.End(); iter++)
    {
        if (i == no)
        {
            csmString name = (*iter).First;
            SetExpression(name.GetRawString());
            return;
        }
        i++;
    }
}

// ---------- 重新加载渲染器 ----------
void LAppModel::ReloadRenderer()
{
    DeleteRenderer();
    CreateRenderer(LAppDelegate::GetInstance()->GetWindowWidth(), LAppDelegate::GetInstance()->GetWindowHeight());
    SetupTextures();
}

// ---------- 设置纹理 ----------
void LAppModel::SetupTextures()
{
    for (csmInt32 modelTextureNumber = 0; modelTextureNumber < _modelSetting->GetTextureCount(); modelTextureNumber++)
    {
        if (strcmp(_modelSetting->GetTextureFileName(modelTextureNumber), "") == 0)
            continue;
        csmString texturePath = _modelSetting->GetTextureFileName(modelTextureNumber);
        texturePath = _modelHomeDir + texturePath;
        LAppTextureManager::TextureInfo *texture = LAppDelegate::GetInstance()->GetTextureManager()->CreateTextureFromPngFile(texturePath.GetRawString());
        GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->BindTexture(modelTextureNumber, texture->id);
    }
#ifdef PREMULTIPLIED_ALPHA_ENABLE
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(true);
#else
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(false);
#endif
}

// ---------- 事件回调 ----------
void LAppModel::MotionEventFired(const csmString &eventValue)
{
    CubismLogInfo("%s is fired on LAppModel!!", eventValue.GetRawString());
}

// ---------- 获取渲染缓冲区 ----------
Csm::Rendering::CubismRenderTarget_OpenGLES2 &LAppModel::GetRenderBuffer()
{
    return _renderBuffer;
}

// ---------- 检查 MOC3 一致性 ----------
csmBool LAppModel::HasMocConsistencyFromFile(const csmChar *mocFileName)
{
    CSM_ASSERT(strcmp(mocFileName, ""));
    csmByte *buffer;
    csmSizeInt size;
    csmString path = mocFileName;
    path = _modelHomeDir + path;
    buffer = CreateBuffer(path.GetRawString(), &size);
    csmBool consistency = CubismMoc::HasMocConsistencyFromUnrevivedMoc(buffer, size);
    if (!consistency)
        CubismLogInfo("Inconsistent MOC3.");
    else
        CubismLogInfo("Consistent MOC3.");
    DeleteBuffer(buffer);
    return consistency;
}

// ---------- 额外动作 / 表情管理 ----------
void LAppModel::LoadExtraFiles()
{
    if (!_extraFileManager)
        return;
    _extraFileManager->loadExtraFiles(QString::fromUtf8(_modelHomeDir.GetRawString()));

    // 加载额外表情
    auto expressions = _extraFileManager->getAvailableExpressions();
    for (const QString &expr : expressions)
    {
        QByteArray data = _extraFileManager->getExpressionData(expr);
        if (data.isEmpty())
            continue;
        ACubismMotion *motion = LoadExpression(
            reinterpret_cast<const csmByte *>(data.constData()), data.size(), expr.toUtf8().constData());
        if (motion)
        {
            csmString key(expr.toUtf8().constData());
            if (extraExpressions[key])
                ACubismMotion::Delete(extraExpressions[key]);
            extraExpressions[key] = motion;
        }
    }

    // 加载额外动作
    auto motions = _extraFileManager->getAvailableMotions();
    for (const QString &motionName : motions)
    {
        QByteArray data = _extraFileManager->getMotionData(motionName);
        if (data.isEmpty())
            continue;
        CubismMotion *motion = static_cast<CubismMotion *>(LoadMotion(
            reinterpret_cast<const csmByte *>(data.constData()), data.size(),
            motionName.toUtf8().constData(), nullptr, nullptr, nullptr, "Extra", 0, _motionConsistency));
        if (motion)
        {
            motion->SetEffectIds(_eyeBlinkIds, _lipSyncIds);
            csmString key(motionName.toUtf8().constData());
            if (extraMotions[key])
                ACubismMotion::Delete(extraMotions[key]);
            extraMotions[key] = motion;
        }
    }

    qDebug() << "[LApp] Loaded" << extraExpressions.GetSize() << "extra expressions and"
             << extraMotions.GetSize() << "extra motions";
    ExtraMotionManager::getInstance()->setModel(this);
}

Csm::CubismMotionQueueEntryHandle LAppModel::StartExtraMotion(const csmChar *motionName, csmInt32 priority,
                                                              ACubismMotion::FinishedMotionCallback onFinished, ACubismMotion::BeganMotionCallback onBegan)
{
    csmString key(motionName);
    if (extraMotions[key])
    {
        ACubismMotion *motion = extraMotions[key];
        motion->SetBeganMotionHandler(onBegan);
        motion->SetFinishedMotionHandler(onFinished);
        if (priority == PriorityForce)
        {
            _motionManager->SetReservePriority(priority);
        }
        else if (!_motionManager->ReserveMotion(priority))
        {
            qDebug() << "[LApp] Can't start extra motion - priority reserved";
            return InvalidMotionQueueEntryHandleValue;
        }
        return _motionManager->StartMotionPriority(motion, false, priority);
    }
    qDebug() << "[LApp] Extra motion not found:" << motionName;
    return InvalidMotionQueueEntryHandleValue;
}

void LAppModel::SetExtraExpression(const csmChar *expressionID)
{
    csmString key(expressionID);
    if (extraExpressions[key] && _expressionManager)
    {
        _expressionManager->StartMotion(extraExpressions[key], false);
    }
    else
    {
        qDebug() << "[LApp] Extra expression not found:" << expressionID;
    }
}

bool LAppModel::HasMotion(const csmChar *motionName) const
{
    csmString key(motionName);
    for (auto it = _motions.Begin(); it != _motions.End(); ++it)
        if (it->First == key && it->Second)
            return true;
    for (auto it = extraMotions.Begin(); it != extraMotions.End(); ++it)
        if (it->First == key && it->Second)
            return true;
    return false;
}

bool LAppModel::HasExpression(const csmChar *expressionID) const
{
    csmString key(expressionID);
    for (auto it = _expressions.Begin(); it != _expressions.End(); ++it)
        if (it->First == key && it->Second)
            return true;
    for (auto it = extraExpressions.Begin(); it != extraExpressions.End(); ++it)
        if (it->First == key && it->Second)
            return true;
    return false;
}
bool LAppModel::HasExtraMotion(const csmChar *motionName) const
{
    csmString key(motionName);
    // 改用迭代器遍历，避免 const 下的 operator[] 错误
    for (auto it = extraMotions.Begin(); it != extraMotions.End(); ++it)
    {
        if (it->First == key && it->Second != nullptr)
            return true;
    }
    return false;
}

bool LAppModel::HasExtraExpression(const csmChar *expressionID) const
{
    csmString key(expressionID);
    for (auto it = extraExpressions.Begin(); it != extraExpressions.End(); ++it)
    {
        if (it->First == key && it->Second != nullptr)
            return true;
    }
    return false;
}