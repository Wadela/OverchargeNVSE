#include "BSUtilities.hpp"
#include "Gamebryo/NiDefaultAVObjectPalette.hpp"
#include "Gamebryo/NiCamera.hpp"
#include "Gamebryo/NiControllerManager.hpp"
#include "Gamebryo/NiD3DUtility.hpp"
#include "Gamebryo/NiMatrix3.hpp"
#include "Gamebryo/NiPlane.hpp"
#include "Gamebryo/NiPoint3.hpp"
#include "Gamebryo/NiRenderer.hpp"
#include "numbers"

// GAME - 0x4B15E0
float BSUtilities::GetAngleDelta(float afStartAngle, float afTargetAngle, float& arfOutDir) noexcept {
	constexpr float fPI = std::numbers::pi_v<float>;
	constexpr float f2PI = 2.f * fPI;

	const float fDelta = afTargetAngle - afStartAngle;
	arfOutDir = 0.f;
	if (fDelta == 0.f)
		return fDelta;
	arfOutDir = 1.f;
	if (fDelta < 0.f) {
		if (fDelta <= -fPI)
			return (fDelta + f2PI);
		else
			arfOutDir = -1.f;
	}
	else if (fDelta > fPI) {
		arfOutDir = -1.f;
		return (f2PI - fDelta);
	}
	return fDelta;
}

float BSUtilities::FLerp(float afNewMin, float afNewMax, float afOldMin, float afOldMax, float afOldValue) noexcept {
	return ((afOldValue - afOldMin) / (afOldMax - afOldMin) * (afNewMax - afNewMin) + afNewMin);
}

// GAME - 0x4E44F0
void BSUtilities::Clamp(float& arfOut, float afMin, float afMax) noexcept {
	if (afMax < afMin)
		return;

	if (afMax < arfOut)
		arfOut = std::fmod(afMax - afMin, arfOut - afMin) + afMin;
	else if (afMin > arfOut)
		arfOut = std::fmod(arfOut - afMin, afMax - afMin) + afMax;
}

// GAME - 0x49E390
bool BSUtilities::CompareFloat(float a, float b, float afMargin) noexcept {
	return afMargin >= std::fabs(a - b);
}

// GAME - 0x49E2F0
// GECK - 0x512130
bool BSUtilities::ComparePoints(const NiPoint3& __restrict a, const NiPoint3& __restrict b, float afMargin) noexcept {
#if USE_DXMATH
	const DirectX::XMVECTOR kAbs = DirectX::XMVectorAbsFast(DirectX::XMVectorSubtract(DirectX::XMLoadNiPoint3(a), DirectX::XMLoadNiPoint3(b)));
	const DirectX::XMVECTOR kMargin = DirectX::XMVectorReplicate(afMargin);
	return DirectX::XMVector3GreaterOrEqual(kMargin, kAbs);
#else
	return CompareFloat(a.x, b.x, afMargin) && CompareFloat(a.y, b.y, afMargin) && CompareFloat(a.z, b.z, afMargin);
#endif
}

// GAME - 0x4AAE30
// GECK - 0x51F8A0
NiAVObject* BSUtilities::GetObjectByName(const NiAVObject* apScene, const char* apName) {
	return GetObjectByName(apScene, apName, true);
}

// GAME - 0xC4B310
// GECK - 0x9D2000
NiAVObject* BSUtilities::GetObjectByName(const NiAVObject* apScene, const char* apName, bool abTestScenegraph) {
	if (!apScene || !apName)
		return nullptr;

	NiControllerManager* pControllerManager = apScene->GetController<NiControllerManager>();
	NiFixedString kName = apName;
	if (pControllerManager) {
		NiAVObject* pObject = pControllerManager->m_spObjectPalette->GetAVObject(kName);
		if (pObject)
			return pObject;
	}

	if (abTestScenegraph) {
		NiAVObject* pObject = apScene->GetObjectByName(kName);
		if (pObject)
			return pObject;
	}

	return nullptr;
}

// GAME - 0x4ADE00
// GECK - N/A
NiAVObject* BSUtilities::GetObjectByName(const NiAVObject* apScene, const NiFixedString& arName) {
	return GetObjectByName(apScene, arName, true);
}

// GAME - 0xC4B470
// GECK - 0x9D2160
NiAVObject* BSUtilities::GetObjectByName(const NiAVObject* apScene, const NiFixedString& arName, bool abTestScenegraph) {
	if (!apScene || !arName)
		return nullptr;

	NiControllerManager* pControllerManager = apScene->GetController<NiControllerManager>();
	if (pControllerManager) {
		NiAVObject* pObject = pControllerManager->m_spObjectPalette->GetAVObject(arName);
		if (pObject)
			return pObject;
	}

	if (abTestScenegraph) {
		NiAVObject* pObject = apScene->GetObjectByName(arName);
		if (pObject)
			return pObject;
	}

	return nullptr;
}

// GAME - 0xAF4250
int32_t BSUtilities::GetMeshesPath(const char* apSource, char* apTarget, uint32_t auiBufferSize) {
	const uint32_t uiLength = strlen(apSource);
	if (uiLength > 3 && !_stricmp(&apSource[uiLength - 3], "spt")) {
		strcpy_s(apTarget, auiBufferSize, "Trees");
		return strcat_s(apTarget, auiBufferSize, apSource);
	}
	else if (!_strnicmp(apSource, "meshes\\", 7u)) {
		return strcpy_s(apTarget, auiBufferSize, apSource);
	}
	else {
		strcpy_s(apTarget, auiBufferSize, "meshes\\");
		return strcat_s(apTarget, auiBufferSize, apSource);
	}
}

char BSUtilities::HasMorpherController(NiNode* node)
{
	return CdeclCall<char>(0x4B5BF0, node);
}