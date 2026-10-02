#pragma once

#include <QtGlobal>
#include <QLibrary>

#define PELR_CUBISM_CORE_FUNCS(X)              \
    X(csmGetVersion)                           \
    X(csmGetLatestMocVersion)                  \
    X(csmGetMocVersion)                        \
    X(csmHasMocConsistency)                    \
    X(csmGetLogFunction)                       \
    X(csmSetLogFunction)                       \
    X(csmReviveMocInPlace)                     \
    X(csmGetSizeofModel)                       \
    X(csmInitializeModelInPlace)               \
    X(csmUpdateModel)                          \
    X(csmGetRenderOrders)                      \
    X(csmReadCanvasInfo)                       \
    X(csmGetParameterCount)                    \
    X(csmGetParameterIds)                      \
    X(csmGetParameterTypes)                    \
    X(csmGetParameterMinimumValues)            \
    X(csmGetParameterMaximumValues)            \
    X(csmGetParameterDefaultValues)            \
    X(csmGetParameterValues)                   \
    X(csmGetParameterRepeats)                  \
    X(csmGetParameterKeyCounts)                \
    X(csmGetParameterKeyValues)                \
    X(csmGetPartCount)                         \
    X(csmGetPartIds)                           \
    X(csmGetPartOpacities)                     \
    X(csmGetPartParentPartIndices)             \
    X(csmGetPartOffscreenIndices)              \
    X(csmGetDrawableCount)                     \
    X(csmGetDrawableIds)                       \
    X(csmGetDrawableConstantFlags)             \
    X(csmGetDrawableDynamicFlags)              \
    X(csmGetDrawableBlendModes)                \
    X(csmGetDrawableTextureIndices)            \
    X(csmGetDrawableDrawOrders)                \
    X(csmGetDrawableOpacities)                 \
    X(csmGetDrawableMaskCounts)                \
    X(csmGetDrawableMasks)                     \
    X(csmGetDrawableVertexCounts)              \
    X(csmGetDrawableVertexPositions)           \
    X(csmGetDrawableVertexUvs)                \
    X(csmGetDrawableIndexCounts)               \
    X(csmGetDrawableIndices)                   \
    X(csmGetDrawableMultiplyColors)            \
    X(csmGetDrawableScreenColors)              \
    X(csmGetDrawableParentPartIndices)         \
    X(csmResetDrawableDynamicFlags)            \
    X(csmGetOffscreenCount)                    \
    X(csmGetOffscreenBlendModes)               \
    X(csmGetOffscreenOpacities)                \
    X(csmGetOffscreenOwnerIndices)             \
    X(csmGetOffscreenMultiplyColors)           \
    X(csmGetOffscreenScreenColors)             \
    X(csmGetOffscreenMaskCounts)               \
    X(csmGetOffscreenMasks)                    \
    X(csmGetOffscreenConstantFlags)

#define PELR_CUBISM_CORE_DECL(name) extern QFunctionPointer p_##name;
PELR_CUBISM_CORE_FUNCS(PELR_CUBISM_CORE_DECL)
#undef PELR_CUBISM_CORE_DECL

class CubismCoreLoader
{
public:
    static CubismCoreLoader &instance();

    bool load();
    bool isLoaded() const { return m_ok; }

private:
    CubismCoreLoader() = default;

    bool resolveAll();
    void clearAll();

    QLibrary m_lib;
    bool m_ok = false;
};
