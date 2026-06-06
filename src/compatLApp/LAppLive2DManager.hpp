#pragma once

#include <CubismFramework.hpp>
#include <Math/CubismMatrix44.hpp>
#include <Type/csmVector.hpp>
#include <string>

class LAppModel;

class LAppLive2DManager
{
public:
    static LAppLive2DManager *GetInstance();
    static void ReleaseInstance();

    void SetUpModel();
    Csm::csmVector<Csm::csmString> GetModelDir() const;
    Csm::csmInt32 GetModelDirSize() const;
    LAppModel *GetModel(Csm::csmUint32 no) const;

    // 设置所有模型的渲染目标尺寸
    void SetRenderTargetSize(Csm::csmUint32 width, Csm::csmUint32 height);

    void ReleaseAllModel();
    void OnDrag(Csm::csmFloat32 x, Csm::csmFloat32 y) const;
    void OnTap(Csm::csmFloat32 x, Csm::csmFloat32 y);
    void OnUpdate() const;
    void NextScene();
    void ChangeScene(Csm::csmInt32 index);
    Csm::csmUint32 GetModelNum() const;
    void SetViewMatrix(Live2D::Cubism::Framework::CubismMatrix44 *m);

    static void SetDragStrength(Csm::csmFloat32 strength);

    // ---------- 自定义方法 ----------
    void StartLipSync(const Csm::csmString &wavFilePath);
    void StartLipSync(Csm::csmUint32 modelIndex, const Csm::csmString &wavFilePath);

    void LoadModelFromPath(const std::string &modelPath, const std::string &fileName);
    void LoadModelFromConfig();

    LAppLive2DManager();
    virtual ~LAppLive2DManager();

private:
    static Csm::csmFloat32 _dragStrength;

    Csm::CubismMatrix44 *_viewMatrix;
    Csm::csmVector<LAppModel *> _models;
    Csm::csmInt32 _sceneIndex;
    Csm::csmVector<Csm::csmString> _modelDir;
};