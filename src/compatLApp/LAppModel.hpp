#pragma once

#include <GL/glew.h>
#include <CubismFramework.hpp>
#include <ICubismModelSetting.hpp>
#include <Type/csmRectF.hpp>
#include <Rendering/OpenGL/CubismRenderTarget_OpenGLES2.hpp>
#include "LAppWavFileHandler_Common.hpp"
#include "LAppModel_Common.hpp"
#include "ExtraFileManager.h"
#include "TtsLipSync.hpp"

/**
 * @brief 自定义模型类，支持额外动作/表情
 */
using namespace Csm;
class LAppModel : public LAppModel_Common
{
public:
    LAppModel();
    virtual ~LAppModel();

    void LoadAssets(const Csm::csmChar *dir, const Csm::csmChar *fileName);
    void ReloadRenderer();
    void Update();
    void Draw(Csm::CubismMatrix44 &matrix);

    Csm::CubismMotionQueueEntryHandle StartMotion(const Csm::csmChar *group, Csm::csmInt32 no, Csm::csmInt32 priority,
                                                  Csm::ACubismMotion::FinishedMotionCallback onFinished = nullptr,
                                                  Csm::ACubismMotion::BeganMotionCallback onBegan = nullptr);
    Csm::CubismMotionQueueEntryHandle StartRandomMotion(const Csm::csmChar *group, Csm::csmInt32 priority,
                                                        Csm::ACubismMotion::FinishedMotionCallback onFinished = nullptr,
                                                        Csm::ACubismMotion::BeganMotionCallback onBegan = nullptr);

    void SetExpression(const Csm::csmChar *expressionID);
    void SetRandomExpression();
    virtual void MotionEventFired(const Live2D::Cubism::Framework::csmString &eventValue);
    virtual Csm::csmBool HitTest(const Csm::csmChar *hitAreaName, Csm::csmFloat32 x, Csm::csmFloat32 y);

    Csm::Rendering::CubismRenderTarget_OpenGLES2 &GetRenderBuffer();
    Csm::csmBool HasMocConsistencyFromFile(const Csm::csmChar *mocFileName);

    // ---------- 额外动作/表情 ----------
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion *> extraMotions;
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion *> extraExpressions;

    Csm::CubismMotionQueueEntryHandle StartExtraMotion(const csmChar *motionName, csmInt32 priority,
                                                       Csm::ACubismMotion::FinishedMotionCallback onFinished = nullptr,
                                                       Csm::ACubismMotion::BeganMotionCallback onBegan = nullptr);
    void SetExtraExpression(const csmChar *expressionID);
    void LoadExtraFiles();

    bool HasMotion(const csmChar *motionName) const;
    bool HasExpression(const csmChar *expressionID) const;
    bool HasExtraMotion(const csmChar *motionName) const;
    bool HasExtraExpression(const csmChar *expressionID) const;

    void StartLipSync(const Csm::csmString &filePath);
    void StopLipSync();

protected:
    void DoDraw();

private:
    void SetupModel(Csm::ICubismModelSetting *setting);
    void SetupTextures();
    void setMouthParameter(Csm::csmFloat32 value);
    void PreloadMotionGroup(const Csm::csmChar *group);
    void ReleaseMotionGroup(const Csm::csmChar *group) const;
    void ReleaseMotions();
    void ReleaseExpressions();

    Csm::ICubismModelSetting *_modelSetting;
    Csm::csmString _modelHomeDir;
    Csm::csmFloat32 _userTimeSeconds;
    Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds;
    Csm::csmVector<Csm::CubismIdHandle> _lipSyncIds;
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion *> _motions;
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion *> _expressions;
    Csm::csmVector<Csm::csmRectF> _hitArea;
    Csm::csmVector<Csm::csmRectF> _userArea;
    Csm::csmBool _motionUpdated; ///< モーション更新フラグ
    const Csm::CubismId *_idParamAngleX;
    const Csm::CubismId *_idParamAngleY;
    const Csm::CubismId *_idParamAngleZ;
    const Csm::CubismId *_idParamBodyAngleX;
    const Csm::CubismId *_idParamEyeBallX;
    const Csm::CubismId *_idParamEyeBallY;
    const Csm::CubismId *_idParamMouthOpen;

    LAppWavFileHandler_Common _wavFileHandler;
    Csm::Rendering::CubismRenderTarget_OpenGLES2 _renderBuffer;

    TtsLipSync _ttsSync;

    ExtraFileManager *_extraFileManager; // 额外文件管理器
};