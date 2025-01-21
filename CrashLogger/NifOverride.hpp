#pragma once

#include "MainHeader.hpp"
#include "Logging.h"
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

	static case_insensitive_set editorIds = {
		"WeapPlasmaRifle",
		"WeapNVPlasmaRifleUnique",
		"PlasmaProjectile",
		"PlasmaProjectile02",
		"PlasmaProjectileQ35",
		"PlasmaCasterProjectile"
	};

	static case_insensitive_set extraModels = {
		"Effects\\MuzzleFlashes\\PlasmaRifleMuzzleFlash.NIF",
		"mps\\mpsplasmaprojectile.nif",
		"effects\\impactenergygreen01.nif",
		"effects\\impactenergybase01.nif",
		"mps\\mpsenergyimpactgreen.nif",
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
				NiColorA col = modelData->m_pkColor[i].Shifted(NiColor(1, 0, 0), 1);
				col.a = modelData->m_pkColor[i].a;

				modelData->m_pkColor[i] = col;
			}

			NiDX9Renderer::GetSingleton()->LockPrecacheCriticalSection();
			NiDX9Renderer::GetSingleton()->PurgeGeometryData(modelData); 
			NiDX9Renderer::GetSingleton()->UnlockPrecacheCriticalSection();
		}
	}

	//Edit Color Modifiers - For preparing particles to have emissive colors pop out more
	static void UpdateColorMod(NiParticleSystem* childParticle)
	{
		if (!childParticle->m_kProperties.m_spMaterialProperty) return;

		childParticle->m_kProperties.m_spMaterialProperty->m_emit = NiColor(1, 0, 0);

		for (NiPSysModifier* it : childParticle->m_kModifierList)
		{
			if (BSPSysSimpleColorModifier* colorMod = it->NiDynamicCast<BSPSysSimpleColorModifier>())
			{
				colorMod->kColor2 = NiColorA(1, 0, 0, 1); 
			}
		}
	}

	//Update Child Particles to all be prepared for emissive color control
	static void PrepParticleColor(const NiNode* node)
	{
		if (BSValueNode* const valueNode = node->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->kData.ucFlags.GetBit(1))
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					if (NiNode* const mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>())
					{
						for (int i = 0; i < mpsNode->m_kChildren.m_usSize; ++i)
						{
							if (NiParticleSystem* const childParticle = mpsNode->m_kChildren.m_pBase[i]->NiDynamicCast<NiParticleSystem>())
							{
								UpdateColorMod(childParticle);
							}
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
			}
		}
	}

	static NiNode* __fastcall ModelLoaderLoadFile(const uint8_t* model, const char* filePath)
	{
		if (filePath && (definedModels.contains(filePath) || strstr(filePath, "Flash")))
		{
			NiNode* node = ThisStdCall<NiNode*>(0x43B230, model); 
			ProcessNiNode(node);
			return node;
		}
		return ThisStdCall<NiNode*>(0x43B230, model);
	}

	static void __fastcall ModelModel(const uint8_t* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV)
	{
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);
		ModelLoaderLoadFile(thisPtr, modelPath);
	}

	inline void __fastcall MuzzleFlashEnable(uint8_t* flash)
	{
		// MuzzleFlash::spLight
		(*reinterpret_cast<NiPointLight**>(flash + 0x10))->SetDiffuseColor(NiColor(0, 0, 1));

		// MuzzleFlash::Enable
		ThisStdCall(0x9BB690, flash);
	}

	inline void Hook()
	{
		WriteRelCall(0x43AB4C, &ModelModel);
		WriteRelCall(0x9BB7CD, &MuzzleFlashEnable);
	}

	inline void PostLoad() {
		// Add any extra models
		for (const std::string elem : extraModels)
		{
			definedModels.insert(elem);
		}
	}
}