#pragma once

#include "BSShaderAccumulator.hpp"
#include "BSShaderManager.hpp"
#include "BSCullingProcess.hpp"
#include "NiCamera.hpp"

class BSCullingProcess;

class BSShaderUtil {
public:
	static void AccumulateScene(const NiCamera* apCamera, NiAVObject* apNode, BSCullingProcess* apCullingProcess);
	static void AccumulateSceneList(const NiCamera* apCamera, void* apGeometryList, BSCullingProcess* apCullingProcess);

	static void RenderScene(NiCamera* apCamera, BSShaderAccumulator* apAccumulator);
	static void RenderScenePreResolveDepth(NiCamera* apCamera, BSShaderAccumulator* apAccumulator);
	static void RenderScenePostResolveDepth(NiCamera* apCamera, BSShaderAccumulator* apAccumulator);

	static NiSourceTexture* ConvertTexture(NiSourceTexture* apTexture, const NiPixelFormat& arFormat);
};