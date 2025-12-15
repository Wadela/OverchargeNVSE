#pragma once

class NiPoint3;
class NiTransform;
class NiSkinInstance;
class NiAVObject;
class NiNode;
class NiCamera;
class NiPlane;
class NiFixedString;
struct D3DXMATRIX;

class BSUtilities {
public:
    static float GetAngleDelta(float afStartAngle, float afTargetAngle, float& arfOutDir) noexcept;

    static float FLerp(float afNewMin, float afNewMax, float afOldMin, float afOldMax, float afOldValue) noexcept;

    static void Clamp(float& arfOut, float afMin, float afMax) noexcept;

    static bool CompareFloat(float a, float b, float afMargin) noexcept;

    static bool ComparePoints(const NiPoint3& __restrict a, const NiPoint3& __restrict b, float afMargin) noexcept;

    static NiAVObject* GetObjectByName(const NiAVObject* apScene, const char* apName);

    static NiAVObject* GetObjectByName(const NiAVObject* apScene, const char* apName, bool abTestScenegraph);

    static NiAVObject* GetObjectByName(const NiAVObject* apScene, const NiFixedString& arName);

    static NiAVObject* GetObjectByName(const NiAVObject* apScene, const NiFixedString& arName, bool abTestScenegraph);

    static int32_t GetMeshesPath(const char* apSource, char* apTarget, uint32_t auiBufferSize);

    static char HasMorpherController(NiNode* node);
};

// GAME - 0x40EBD0
template <typename A, typename B>
B BSMin(A left, B right) {
    if (right <= left)
        return right;
    else
        return left;
}

// GAME - 0x404010
template <typename A, typename B>
B BSMax(A left, B right) {
    if (right >= left)
        return right;
    else
        return left;
}