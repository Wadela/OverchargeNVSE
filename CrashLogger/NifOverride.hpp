#pragma once

#include "MainHeader.hpp"
#include "Defines.h"

//Bethesda
#include "BSStream.hpp"
#include "BGSAddonNode.hpp"
#include "BSValueNode.hpp"
#include "BSParticleSystemManager.h"
#include "BSMasterParticleSystem.hpp"
#include "BSPSysMultiTargetEmitterCtlr.hpp"
#include "TESDataHandler.hpp"

//Gamebryo
#include "NiObject.hpp"
#include "NiAVObject.hpp"
#include "NiNode.hpp"
#include "NiGeometry.hpp"
#include "NiGeometryData.hpp"
#include "NiFixedString.hpp"
#include "NiStream.hpp"
#include "NiParticleSystem.hpp"
#include "NiTimeController.hpp"
#include "NiPointLight.hpp"
#include <BSPSysSimpleColorModifier.hpp>
#include <MuzzleFlash.hpp>
#include "Model.hpp"
#include "NiCloningProcess.hpp"

#include "Overcharge.hpp"

namespace Overcharge
{
	static void SetEmissiveColor(NiAVObjectPtr obj, const NiColor& color, NiMaterialPropertyPtr newMatProp = nullptr)
	{
		if (!obj) return;
		auto geom = obj->NiDynamicCast<NiGeometry>();
		if (!geom) return;

		auto& matProp = geom->m_kProperties.m_spMaterialProperty;
		if (newMatProp)
			matProp = newMatProp;

		if (matProp)
			matProp->m_emit = color;
	}

	static void FindValueNodes(const NiNode* apNode, std::vector<BSValueNode*>& result) {
		for (int i = 0; i < apNode->m_kChildren.m_usSize; i++) {
			const auto& child = apNode->m_kChildren.m_pBase[i];

			if (const auto pValueNode = child->NiDynamicCast<BSValueNode>()) {
				result.emplace_back(pValueNode);
			}
			else if (const auto pNiNode = child->NiDynamicCast<NiNode>()) {
				FindValueNodes(pNiNode, result);
			}
		}
	}

	static std::vector<BSValueNode*> FindValueNodes(const NiNode* apNode) {
		std::vector<BSValueNode*> valueNodes{};
		FindValueNodes(apNode, valueNodes);
		return valueNodes;
	}

	static void TraverseNiNode(const NiNodePtr obj, NiColor& color)
	{
		for (int i = 0; i < obj->m_kChildren.m_usSize; i++)
		{
			NiAVObject* const child = obj->m_kChildren[i].m_pObject;
			if (child)
			{
				if (child->IsNiType<NiNode>())
				{
					NiNodePtr childNode = static_cast<NiNode*>(child);
					TraverseNiNode(childNode.m_pObject, color);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometryPtr childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveColor(childGeom.m_pObject, color);
				}
			}
		}
	}

	//Edit Color Modifiers - For preparing particles to have emissive colors pop out more
	static void UpdateColorMod(NiParticleSystem* childParticle)
	{
		if (!childParticle) return;

		for (NiPSysModifier* it : childParticle->m_kModifierList)
		{
			if (BSPSysSimpleColorModifier* colorMod = it->NiDynamicCast<BSPSysSimpleColorModifier>())
			{
				NiColorA col = colorMod->kColor2.Shifted(NiColor(0.8f, 0.8f, 0.8f), 1);

				col.a = colorMod->kColor1.a;
				colorMod->kColor1 = col;

				col.a = colorMod->kColor2.a;
				colorMod->kColor2 = col;

				col.a = colorMod->kColor3.a;
				colorMod->kColor3 = col;
			}
		}
	}

	//Edit Vertex Colors - Primarily for preparing meshes to have emissive colors to pop out more 
	static void PrepVertexColor(const NiGeometry* geom)
	{
		if (const NiGeometryDataPtr modelData = geom->m_spModelData; modelData && modelData->m_pkColor) 
		{
			for (int i = 0; i < modelData->m_usVertices; i++)
			{
				NiColorA col = modelData->m_pkColor[i].Shifted(NiColor(0.65f, 0.65f, 0.65f), 1);
				col.a = modelData->m_pkColor[i].a;

				modelData->m_pkColor[i] = col;
			}
		}
	}

	//Iterates through node children to guide children appropriately
	static void ProcessModel(const NiNode* obj)
	{
		for (int i = 0; i < obj->m_kChildren.m_usSize; i++)
		{
			NiAVObject* const child = obj->m_kChildren[i].m_pObject;

			if (child) 
			{
				if (child->IsNiType<NiParticleSystem>())
				{
					NiParticleSystem* childPsys = static_cast<NiParticleSystem*>(child);
					UpdateColorMod(childPsys);
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessModel(childNode);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					PrepVertexColor(childGeom);
				}
			}
		}
	}
	 
	static NiNode* __fastcall ModelLoaderLoadFile(const Model* model, const char* filePath) 
	{
		if (filePath && (definedModels.contains(filePath)))
		{
			NiNode* node = model->spNode;
			ProcessModel(node);
			return node;
		}
		return model->spNode;
	}

	static void __fastcall ModelModel(const Model* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV) 
	{
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);

		if (g_OCSettings.iEnableVisualEffects > 0)
		ModelLoaderLoadFile(thisPtr, modelPath); 
	}

	inline void Hook()
	{
		WriteRelCall(0x43AB4C, &ModelModel);
	}

	inline void PostLoad() {
		// Add any extra models
		for (const std::string elem : extraModels)
		{
			definedModels.insert(elem); 
		}
	}
} 