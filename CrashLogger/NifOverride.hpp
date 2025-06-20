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
	static void ShowSceneGraph(NiAVObject* obj)
	{
		const void* tree = new uint8_t[0x30];
		TESMain* tesMain = TESMain::GetSingleton();
		const char* const name = obj->GetName() ? obj->GetName() : "<null>";

		ThisStdCall(0x4D61B0, tree, tesMain->hInstance, tesMain->hWnd, obj, name, 0x80000000, 0x80000000, 800, 600);
	}

	//Vertex colored nodes to change
	const case_insensitive_set vertexColors = {
		"##PLRGlassTube:0",
		"CoreWispyEnergy02:0",
		"CoreWispyEnergy01:1",
		"CoreHot01:1",
		"Glow:0",
		"HorizontalFlash06",
		"Plane01:0",
		"lasergeometry:0",
		"lasergeometry:1",
		"planeburst:0",
		"planeburst:1",
		"BigGlow:0"
	};

	//NiMaterialProperties to change
	const case_insensitive_set matProps = {
		"##PLRGlassTube:0",
		"Plane02:0",
		"CoreWispyEnergy02:0",
		"CoreWispyEnergy03",
		"HorizontalFlash06",
		"Plane01:0",
		"Glow:0",
		"CoreHot01:1",
		"pWisps01",
		"pShockTrail",
		"pEnergyHit",
		"pRingImpact",
		"lasergeometry:0",
		"lasergeometry:1",
		"planeburst:0",
		"planeburst:1",
		"BigGlow:0"
	};

	static case_insensitive_set extraModels = {
		"Effects\\MuzzleFlashes\\PlasmaRifleMuzzleFlash.NIF",
		"mps\\mpsplasmaprojectile.nif",
		"effects\\impactenergygreen01.nif",
		"effects\\impactenergygreenflesh01.nif",
		"effects\\impactenergybase01.nif",
		"projectiles\\plasmaprojectile01.nif",
		"projectiles\\testlaserbeamsteady.nif",
		"mps\\mpsenergyimpactred.nif",
		"Effects\\MuzzleFlashes\\laserriflemuzzleflash.NIF",
		"weapons\\2handrifle\\plasmarifle.nif"
	};

	//Auto populated based on editorIds above
	static case_insensitive_set definedModels{};

	//Edit Vertex Colors - Primarily for preparing meshes to have emissive colors to pop out more 
	static void PrepVertexColor(const NiGeometry* geom)
	{
		if (const NiGeometryDataPtr modelData = geom->m_spModelData; modelData && modelData->m_pkColor) 
		{
			for (int i = 0; i < modelData->m_usVertices; i++)
			{
				NiColorA col = modelData->m_pkColor[i].Shifted(NiColor(0.8f, 0.8f, 0.8f), 1);
				col.a = modelData->m_pkColor[i].a;

				modelData->m_pkColor[i] = col;
			}
		}
	}

	static void PrepVertexColorData(NiGeometryData* modelData)
	{
		for (int i = 0; i < modelData->m_usVertices; i++)
		{
			NiColorA col = modelData->m_pkColor[i].Shifted(NiColor(0.8f, 0.8f, 0.8f), 1);
			col.a = modelData->m_pkColor[i].a;

			modelData->m_pkColor[i] = col;
		}
	}

	//Edit Color Modifiers - For preparing particles to have emissive colors pop out more
	static void UpdateColorMod(NiParticleSystem* childParticle)
	{
		if (!childParticle->m_kProperties.m_spMaterialProperty) return;

		childParticle->m_kProperties.m_spMaterialProperty->m_emit = NiColor(0.8f, 0.8f, 0.8f);

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

	//Update Child Particles to all be prepared for emissive color control
	static void PrepParticleColor(const NiNode* node)
	{
		if (BSValueNode* const valueNode = node->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->uiIndex)
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					for (int i = 0; i < mps->kChildParticles.m_usSize; i++)
					{
						if (NiNode* const mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>())
						{
							for (int i = 0; i < mpsNode->m_kChildren.m_usSize; i++)
							{
								if (NiParticleSystem* const childParticle = mpsNode->m_kChildren.m_pBase[i]->NiDynamicCast<NiParticleSystem>())
								{
									UpdateColorMod(childParticle);
								}
							}
						}
						if (NiParticleSystem* mpsGeom = mps->kChildParticles.m_pBase[i]->NiDynamicCast<NiParticleSystem>())  
						{
							UpdateColorMod(mpsGeom);
						}
					}
				}
			}
		}
	}

	//Iterates through node children to guide children appropriately
	static void ProcessNiNode(const NiNode* obj)
	{
		for (int i = 0; i < obj->m_kChildren.m_usSize; i++)
		{
			NiAVObject* const child = obj->m_kChildren[i].m_pObject;

			if (child) 
			{
				//Checks if RTTI comparison is valid before static casting to avoid dynamic casting every single time
				if (child->IsNiType<NiParticleSystem>())
				{
					NiParticleSystem* childPsys = static_cast<NiParticleSystem*>(child);
					UpdateColorMod(childPsys);
				}
				else if (child->IsNiType<BSValueNode>())
				{
					BSValueNode* childValNode = static_cast<BSValueNode*>(child);
					PrepParticleColor(childValNode); 
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessNiNode(childNode);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					PrepVertexColor(childGeom);
				}
				else if (child->IsNiType<NiGeometryData>())
				{
					NiGeometryData* childGeomData = child->NiDynamicCast<NiGeometryData>();
					PrepVertexColorData(childGeomData);
				}
			}
		}
	}
	 
	static NiNode* __fastcall ModelLoaderLoadFile(const Model* model, const char* filePath) 
	{
		if (filePath && (definedModels.contains(filePath) || strstr(filePath, "Flash")))
		{
			NiNode* node = model->spNode;
			ProcessNiNode(node);
			return node;
		}
		return model->spNode;
	}

	static void __fastcall ModelModel(const Model* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV) 
	{
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);
		ModelLoaderLoadFile(thisPtr, modelPath); 
	}

	inline void Hook()
	{
		//WriteRelCall(0x43AB4C, &ModelModel);
	}

	inline void PostLoad() {
		// Add any extra models
		for (const std::string elem : extraModels)
		{
			definedModels.insert(elem); 
		}
	}
}