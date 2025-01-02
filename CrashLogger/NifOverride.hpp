#pragma once

#include "MainHeader.hpp"
#include "Logging.h"
#include "Defines.h"
#include "NiHeatController.hpp"

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

namespace Overcharge
{
	static void ShowSceneGraph(NiAVObject* obj)
	{
		const void* tree = new uint8_t[0x30];
		const auto tesMain = TESMain::GetSingleton();
		const auto name = obj->GetName() ? obj->GetName() : "<null>";

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
		"projectiles\\testlaserbeamsteady.nif",
		"mps\\mpsenergyimpactred.nif",
		"Effects\\MuzzleFlashes\\laserriflemuzzleflash.NIF",
	};

	// Auto populated based on editorIds above
	static case_insensitive_set definedModels{};

	//Stores geometry data for later use
	static NiGeometryDataPtr PreloadGeometryData(const char* nodeName, const char* path)
	{
		// Cached geometry data
		static std::unordered_map<std::string, NiGeometryDataPtr> cache{};
		const std::string key = std::string(path) + nodeName;
		if (cache.contains(key))
		{
			return cache[key];
		}

		BEGIN_SCOPE();
		log("Manually loading vertex data at node '%s' for mesh '%s'", nodeName, path);
		BEGIN_SCOPE();

		StackObject<BSStream> kStream;
		BSStream::Create(&kStream.Get());
		BSStream* pStream = kStream.GetPtr();
		char filePath[256];
		snprintf(filePath, 256, "%s\\%s", "Meshes", path);

		if (!pStream->Load2(filePath))
		{
			log("Failed to load mesh: %s", path);
			log("Stream error: %s", pStream->m_acLastErrorMessage);
			return nullptr;
		}

		const NiObject* pObject = pStream->GetObjectAt(0);
		if (!pObject)
		{
			log("Failed to load root node of mesh: %s", path);
			return nullptr;
		}

		NiNode* const node = pObject->IsNiNode();
		const NiAVObject* targetNode = node->GetObjectByName(NiFixedString(nodeName));
		if (!targetNode)
		{
			log("Node does not exist");
			return nullptr;
		}

		NiGeometry* const geom = targetNode->IsGeometry();
		if (!geom || !geom->m_spModelData->m_pkColor)
		{
			log("Node is not geometry or does not have vertex colors");
			return nullptr;
		}

		constexpr int ucKeepFlags =
			NiGeometryData::KEEP_XYZ |
			NiGeometryData::KEEP_NORM |
			NiGeometryData::KEEP_INDICES |
			NiGeometryData::KEEP_BONEDATA |
			NiGeometryData::KEEP_COLOR;

		geom->m_spModelData->m_ucKeepFlags = ucKeepFlags;
		cache[key] = geom->m_spModelData->Clone()->NiDynamicCast<NiGeometryData>();

		return geom->m_spModelData;
	}

	static bool HandleNiObject(const NiAVObjectPtr& node, const char* modelPath = nullptr)
	{
		BEGIN_SCOPE();
		log("Handling object: %s", node->GetName());
		bool needsController = false;

		if (NiGeometry* const geom = node->IsGeometry())
		{
			BEGIN_SCOPE();
			log("Processing NiGeometry");

			if (matProps.contains(node->GetName()) && geom->GetMaterialProperty())
			{
				log("Material properties enabled");
				needsController = true;
			}
			else
			{
				log("Skipping material properties.. Not configured");
			}
			if (vertexColors.contains(node->GetName()))
			{
				if (!modelPath)
				{
					log("No model path provided for vertex color update");
				}
				else
				{
					if (const NiGeometryDataPtr newGeomData = PreloadGeometryData(node->GetName(), modelPath))
					{
						geom->SetModelData(newGeomData);
						log("Vertex colors updated");
						needsController = true;
					}
				}
			}
			else
			{
				log("Skipping vertex color update.. Not configured");
			}
		}

		if (const NiParticleSystem* particleSystem = node->NiDynamicCast<NiParticleSystem>())
		{
			log("Processing NiParticleSystem");
			needsController = true;
		}

		return needsController;
	}

	static void ProcessNiNode(const NiAVObjectPtr& node, const char* modelPath = nullptr)
	{
		BEGIN_SCOPE();
		log("Processing node %s", node->GetName());

		bool bAddController = false;

		if (const NiNode* niNode = node->IsNiNode())
		{
			BEGIN_SCOPE();
			log("Is NiNode (%d children)", niNode->m_kChildren.m_usSize);

			for (uint32_t i = 0; i < niNode->m_kChildren.m_usSize; i++)
			{
				if (auto child = niNode->m_kChildren[i])
				{
					ProcessNiNode(child, modelPath);
				}
			}
		}

		bAddController = HandleNiObject(node, modelPath);

		if (bAddController)
		{
			BEGIN_SCOPE();
			log("Adding NiHeatController to %s", node->GetName());
			NiHeatController::CreateObject()->SetTarget(node);
		}
	}

	static void InjectMPS(const NiNodePtr& node)
	{
		BEGIN_SCOPE();
		log("Processing node: %s", node->GetName());

		if (BSValueNode* valueNode = node->NiDynamicCast<BSValueNode>())
		{
			BEGIN_SCOPE();

			const SInt32 addonNodeID = valueNode->iValue;
			const BGSAddonNode* addonNode = TESDataHandler::GetSingleton()->GetAddonNode(addonNodeID);

			log("Addon node value: %d", addonNodeID);
			log("Addon node: 0x%p", addonNode);

			if (addonNode && addonNode->kData.ucFlags.GetBit(1))
			{
				BEGIN_SCOPE();

				const BSParticleSystemManager* manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				log("Particle system ID: %d", particleSystemIndex);

				if (const BSMasterParticleSystem* mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					BEGIN_SCOPE();
					log("MPS: 0x%p", mps);

					if (NiNode* const newNode = mps->GetAt(0)->Clone()->IsNiNode())
					{
						BEGIN_SCOPE();
						log("Grabbed main MPS node: %s", newNode->GetName());

						// Ensure that 3d offsets are correct
						newNode->m_kLocal = valueNode->m_kLocal;
						newNode->m_kWorld = valueNode->m_kWorld;

						// Copy value node children
						for (int i = 0; i < valueNode->m_kChildren.m_usSize; i++)
						{
							if (const NiAVObjectPtr child = valueNode->m_kChildren.m_pBase[i])
							{
								BEGIN_SCOPE();
								log("Attaching value node child: %s", child->GetName());
								newNode->AttachChild(child->Clone()->NiDynamicCast<NiAVObject>(), true);
							}
						}
						node->m_pkParent->AttachChild(newNode, false);

						BEGIN_SCOPE();
						log("Attached new node");
					}
				}
			}
		}
		else
		{
			BEGIN_SCOPE();
			log("Node is not a BSValueNode. Skipping.");

			for (int i = 0; i < node->m_kChildren.m_usSize; i++)
			{
				const auto child = node->m_kChildren[i];
				if (!child)
				{
					continue;
				}

				if (const NiNodePtr childNode = child->IsNiNode())
				{
					InjectMPS(childNode);
				}

				const auto psys = child->NiDynamicCast<NiParticleSystem>();
				if (!psys)
				{
					continue;
				}
				if (const auto bsp = psys->GetController<BSPSysMultiTargetEmitterCtlr>())
				{
					child->RemoveController(bsp);
				}
			}
		}
	}

	static NiNode* __fastcall ModelLoaderLoadFile(const uint8_t* model)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		const char* path = *reinterpret_cast<char**>(ebp + 0x8);

		if (path && (definedModels.contains(path) || strstr(path, "Flash")))
		{
			auto* node = ThisStdCall<NiNode*>(0x43B230, model);

			std::println("Modifying model '{}'", path);

			InjectMPS(node);
			ProcessNiNode(node, path);

			return node;
		}

		return ThisStdCall<NiNode*>(0x43B230, model);
	}

	static void __fastcall ModelModel(const uint8_t* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV)
	{
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);
		ModelLoaderLoadFile(thisPtr);
	}

	inline void __fastcall MuzzleFlashEnable(uint8_t* flash)
	{
		// MuzzleFlash::spLight
		(*reinterpret_cast<NiPointLight**>(flash + 0x10))->SetDiffuseColor(rgbShift(GetTickCount64()));

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
		for (const auto& elem : extraModels)
		{
			definedModels.insert(elem);
		}

		// Lookup specified forms by editor id, requires JG
		for (const auto& formName : editorIds)
		{
			TESForm* const found = CdeclCall<TESForm*>(0x483A00, formName.c_str());
			if (!found || !found->IsObject())
			{
				continue;
			}

			const TESObject* obj = reinterpret_cast<TESObject*>(found);
			if (const auto fName = obj->GetModelFileName())
			{
				definedModels.insert(fName);
				log("Added model %s", fName);
			}
		}
	}
}