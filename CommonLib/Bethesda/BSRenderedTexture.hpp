#pragma once

#include "NiRenderedTexture.hpp"
#include "NiRenderTargetGroup.hpp"
#include "NiRenderedCubeMap.hpp"
#include "BSString.hpp"
#include "NiDX9TextureData.hpp"

NiSmartPointer(BSRenderedTexture);

class BSRenderedTexture : public NiObject {
public:
	NiRenderTargetGroupPtr	spRenderTargetGroups[6];
	NiRenderTargetGroupPtr	spSomeRT;
	NiObjectPtr				spObject024;
	NiObjectPtr				spObject028;
	SInt32					eType; // BSTextureManager::RenderTargetTypes
	NiRenderedTexturePtr	spRenderedTextures[4];

	NiNewRTTI(BSRenderedTexture, NiObject);

	static bool ReleaseUnusedRenderTargets();
	static void ReleaseCurrentRenderTarget();
};

ASSERT_SIZE(BSRenderedTexture, 0x40);