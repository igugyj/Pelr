#include "Live2DCubismCore.h"

#include "cubismcoreloader.hpp"

namespace
{

template <typename PFN>
inline PFN bind(QFunctionPointer p)
{
    return reinterpret_cast<PFN>(p);
}

} // namespace

csmVersion csmGetVersion()
{
    auto fn = bind<csmVersion (*)()>(p_csmGetVersion);
    return fn ? fn() : 0;
}

csmMocVersion csmGetLatestMocVersion()
{
    auto fn = bind<csmMocVersion (*)()>(p_csmGetLatestMocVersion);
    return fn ? fn() : 0;
}

csmMocVersion csmGetMocVersion(const void *address, const unsigned int size)
{
    auto fn = bind<csmMocVersion (*)(const void *, const unsigned int)>(p_csmGetMocVersion);
    return fn ? fn(address, size) : 0;
}

int csmHasMocConsistency(void *address, const unsigned int size)
{
    auto fn = bind<int (*)(void *, const unsigned int)>(p_csmHasMocConsistency);
    return fn ? fn(address, size) : 0;
}

csmLogFunction csmGetLogFunction()
{
    auto fn = bind<csmLogFunction (*)()>(p_csmGetLogFunction);
    return fn ? fn() : nullptr;
}

void csmSetLogFunction(csmLogFunction handler)
{
    auto fn = bind<void (*)(csmLogFunction)>(p_csmSetLogFunction);
    if (fn)
        fn(handler);
}

csmMoc *csmReviveMocInPlace(void *address, const unsigned int size)
{
    auto fn = bind<csmMoc *(*)(void *, const unsigned int)>(p_csmReviveMocInPlace);
    return fn ? fn(address, size) : nullptr;
}

unsigned int csmGetSizeofModel(const csmMoc *moc)
{
    auto fn = bind<unsigned int (*)(const csmMoc *)>(p_csmGetSizeofModel);
    return fn ? fn(moc) : 0;
}

csmModel *csmInitializeModelInPlace(const csmMoc *moc, void *address, const unsigned int size)
{
    auto fn = bind<csmModel *(*)(const csmMoc *, void *, const unsigned int)>(p_csmInitializeModelInPlace);
    return fn ? fn(moc, address, size) : nullptr;
}

void csmUpdateModel(csmModel *model)
{
    auto fn = bind<void (*)(csmModel *)>(p_csmUpdateModel);
    if (fn)
        fn(model);
}

const int *csmGetRenderOrders(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetRenderOrders);
    return fn ? fn(model) : nullptr;
}

void csmReadCanvasInfo(const csmModel *model, csmVector2 *outSizeInPixels,
                       csmVector2 *outOriginInPixels, float *outPixelsPerUnit)
{
    auto fn = bind<void (*)(const csmModel *, csmVector2 *, csmVector2 *, float *)>(p_csmReadCanvasInfo);
    if (fn)
        fn(model, outSizeInPixels, outOriginInPixels, outPixelsPerUnit);
}

int csmGetParameterCount(const csmModel *model)
{
    auto fn = bind<int (*)(const csmModel *)>(p_csmGetParameterCount);
    return fn ? fn(model) : 0;
}

const char **csmGetParameterIds(const csmModel *model)
{
    auto fn = bind<const char **(*)(const csmModel *)>(p_csmGetParameterIds);
    return fn ? fn(model) : nullptr;
}

const csmParameterType *csmGetParameterTypes(const csmModel *model)
{
    auto fn = bind<const csmParameterType *(*)(const csmModel *)>(p_csmGetParameterTypes);
    return fn ? fn(model) : nullptr;
}

const float *csmGetParameterMinimumValues(const csmModel *model)
{
    auto fn = bind<const float *(*)(const csmModel *)>(p_csmGetParameterMinimumValues);
    return fn ? fn(model) : nullptr;
}

const float *csmGetParameterMaximumValues(const csmModel *model)
{
    auto fn = bind<const float *(*)(const csmModel *)>(p_csmGetParameterMaximumValues);
    return fn ? fn(model) : nullptr;
}

const float *csmGetParameterDefaultValues(const csmModel *model)
{
    auto fn = bind<const float *(*)(const csmModel *)>(p_csmGetParameterDefaultValues);
    return fn ? fn(model) : nullptr;
}

float *csmGetParameterValues(csmModel *model)
{
    auto fn = bind<float *(*)(csmModel *)>(p_csmGetParameterValues);
    return fn ? fn(model) : nullptr;
}

const int *csmGetParameterRepeats(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetParameterRepeats);
    return fn ? fn(model) : nullptr;
}

const int *csmGetParameterKeyCounts(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetParameterKeyCounts);
    return fn ? fn(model) : nullptr;
}

const float **csmGetParameterKeyValues(const csmModel *model)
{
    auto fn = bind<const float **(*)(const csmModel *)>(p_csmGetParameterKeyValues);
    return fn ? fn(model) : nullptr;
}

int csmGetPartCount(const csmModel *model)
{
    auto fn = bind<int (*)(const csmModel *)>(p_csmGetPartCount);
    return fn ? fn(model) : 0;
}

const char **csmGetPartIds(const csmModel *model)
{
    auto fn = bind<const char **(*)(const csmModel *)>(p_csmGetPartIds);
    return fn ? fn(model) : nullptr;
}

float *csmGetPartOpacities(csmModel *model)
{
    auto fn = bind<float *(*)(csmModel *)>(p_csmGetPartOpacities);
    return fn ? fn(model) : nullptr;
}

const int *csmGetPartParentPartIndices(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetPartParentPartIndices);
    return fn ? fn(model) : nullptr;
}

const int *csmGetPartOffscreenIndices(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetPartOffscreenIndices);
    return fn ? fn(model) : nullptr;
}

int csmGetDrawableCount(const csmModel *model)
{
    auto fn = bind<int (*)(const csmModel *)>(p_csmGetDrawableCount);
    return fn ? fn(model) : 0;
}

const char **csmGetDrawableIds(const csmModel *model)
{
    auto fn = bind<const char **(*)(const csmModel *)>(p_csmGetDrawableIds);
    return fn ? fn(model) : nullptr;
}

const csmFlags *csmGetDrawableConstantFlags(const csmModel *model)
{
    auto fn = bind<const csmFlags *(*)(const csmModel *)>(p_csmGetDrawableConstantFlags);
    return fn ? fn(model) : nullptr;
}

const csmFlags *csmGetDrawableDynamicFlags(const csmModel *model)
{
    auto fn = bind<const csmFlags *(*)(const csmModel *)>(p_csmGetDrawableDynamicFlags);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableBlendModes(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableBlendModes);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableTextureIndices(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableTextureIndices);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableDrawOrders(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableDrawOrders);
    return fn ? fn(model) : nullptr;
}

const float *csmGetDrawableOpacities(const csmModel *model)
{
    auto fn = bind<const float *(*)(const csmModel *)>(p_csmGetDrawableOpacities);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableMaskCounts(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableMaskCounts);
    return fn ? fn(model) : nullptr;
}

const int **csmGetDrawableMasks(const csmModel *model)
{
    auto fn = bind<const int **(*)(const csmModel *)>(p_csmGetDrawableMasks);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableVertexCounts(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableVertexCounts);
    return fn ? fn(model) : nullptr;
}

const csmVector2 **csmGetDrawableVertexPositions(const csmModel *model)
{
    auto fn = bind<const csmVector2 **(*)(const csmModel *)>(p_csmGetDrawableVertexPositions);
    return fn ? fn(model) : nullptr;
}

const csmVector2 **csmGetDrawableVertexUvs(const csmModel *model)
{
    auto fn = bind<const csmVector2 **(*)(const csmModel *)>(p_csmGetDrawableVertexUvs);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableIndexCounts(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableIndexCounts);
    return fn ? fn(model) : nullptr;
}

const unsigned short **csmGetDrawableIndices(const csmModel *model)
{
    auto fn = bind<const unsigned short **(*)(const csmModel *)>(p_csmGetDrawableIndices);
    return fn ? fn(model) : nullptr;
}

const csmVector4 *csmGetDrawableMultiplyColors(const csmModel *model)
{
    auto fn = bind<const csmVector4 *(*)(const csmModel *)>(p_csmGetDrawableMultiplyColors);
    return fn ? fn(model) : nullptr;
}

const csmVector4 *csmGetDrawableScreenColors(const csmModel *model)
{
    auto fn = bind<const csmVector4 *(*)(const csmModel *)>(p_csmGetDrawableScreenColors);
    return fn ? fn(model) : nullptr;
}

const int *csmGetDrawableParentPartIndices(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetDrawableParentPartIndices);
    return fn ? fn(model) : nullptr;
}

void csmResetDrawableDynamicFlags(csmModel *model)
{
    auto fn = bind<void (*)(csmModel *)>(p_csmResetDrawableDynamicFlags);
    if (fn)
        fn(model);
}

int csmGetOffscreenCount(const csmModel *model)
{
    auto fn = bind<int (*)(const csmModel *)>(p_csmGetOffscreenCount);
    return fn ? fn(model) : 0;
}

const int *csmGetOffscreenBlendModes(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetOffscreenBlendModes);
    return fn ? fn(model) : nullptr;
}

const float *csmGetOffscreenOpacities(const csmModel *model)
{
    auto fn = bind<const float *(*)(const csmModel *)>(p_csmGetOffscreenOpacities);
    return fn ? fn(model) : nullptr;
}

const int *csmGetOffscreenOwnerIndices(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetOffscreenOwnerIndices);
    return fn ? fn(model) : nullptr;
}

const csmVector4 *csmGetOffscreenMultiplyColors(const csmModel *model)
{
    auto fn = bind<const csmVector4 *(*)(const csmModel *)>(p_csmGetOffscreenMultiplyColors);
    return fn ? fn(model) : nullptr;
}

const csmVector4 *csmGetOffscreenScreenColors(const csmModel *model)
{
    auto fn = bind<const csmVector4 *(*)(const csmModel *)>(p_csmGetOffscreenScreenColors);
    return fn ? fn(model) : nullptr;
}

const int *csmGetOffscreenMaskCounts(const csmModel *model)
{
    auto fn = bind<const int *(*)(const csmModel *)>(p_csmGetOffscreenMaskCounts);
    return fn ? fn(model) : nullptr;
}

const int **csmGetOffscreenMasks(const csmModel *model)
{
    auto fn = bind<const int **(*)(const csmModel *)>(p_csmGetOffscreenMasks);
    return fn ? fn(model) : nullptr;
}

const csmFlags *csmGetOffscreenConstantFlags(const csmModel *model)
{
    auto fn = bind<const csmFlags *(*)(const csmModel *)>(p_csmGetOffscreenConstantFlags);
    return fn ? fn(model) : nullptr;
}
