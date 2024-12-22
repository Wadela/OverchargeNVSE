#pragma once
#include <filesystem>
#include <numbers>
#include <print>
#include <set>
#include <unordered_map>

#include "BGSAddonNode.hpp"
#include "BSMasterParticleSystem.hpp"
#include "BSParticleSystemManager.h"
#include "BSPSysMultiTargetEmitterCtlr.hpp"
#include "BSStream.hpp"
#include "BSValueNode.hpp"
#include "Defines.h"
#include "NiCloningProcess.hpp"
#include "NiGeometryData.hpp"
#include "NiNode.hpp"
#include "SafeWrite.hpp"
#include "TESMain.hpp"
#include "NiGeometry.hpp"
#include "NiHeatController.hpp"
#include "NiStream.hpp"
#include "NiLight.hpp"
#include "NiParticleSystem.hpp"
#include "NiPointLight.hpp"
#include "TESDataHandler.hpp"
#include "Logging.h"

namespace Overcharge {
	static void ShowSceneGraph(NiAVObject* obj) {
		const void* tree = new uint8_t[0x30];
		const auto tesMain = TESMain::GetSingleton();
		const auto name = obj->GetName() ? obj->GetName() : "<null>";
		ThisStdCall(0x4D61B0, tree, tesMain->hInstance, tesMain->hWnd, obj, name, 0x80000000, 0x80000000, 800, 600);
	}

	class case_insensitive_set : public std::set<std::string> {
	public:
		case_insensitive_set(std::initializer_list<std::string> items) {
			for (auto& cur : items) {
				insert(cur);
			}
		}

		void insert(std::string data) {
			std::ranges::transform(data, data.begin(),
				[](unsigned char c) {
					return std::tolower(c);
				});
			std::set<std::string>::insert(data);
		}

		bool contains(std::string data) const {
			std::ranges::transform(data, data.begin(),
				[](unsigned char c) {
					return std::tolower(c);
				});
			return std::set<std::string>::contains(data);
		}
	};

	inline UInt8* GetParentBasePtr(void* addressOfReturnAddress, const bool lambda = false) {
		auto* basePtr = static_cast<UInt8*>(addressOfReturnAddress) - 4;
	#if _DEBUG
		// in debug mode, lambdas are wrapped inside a closure wrapper function, so one more step needed
		if (lambda) {
			basePtr = *reinterpret_cast<UInt8**>(basePtr);
		}
	#endif
		return *reinterpret_cast<UInt8**>(basePtr);
	}

	// Vertex colored nodes to change
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

	// NiMaterialProperties to change
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

	static NiGeometryDataPtr PreloadGeometryData(const std::string& nodeName, const std::string& path) {
		// Cached geometry data
		static std::unordered_map<std::string, NiGeometryDataPtr> cache{};
		const auto key = path + nodeName;
		if (cache.contains(key)) {
			return cache[key];
		}

		BEGIN_SCOPE();
		log("Manually loading vertex data at node '%s' for mesh '%s'", nodeName.c_str(), path.c_str());
		BEGIN_SCOPE();

		StackObject<BSStream> kStream;
		BSStream::Create(&kStream.Get());
		BSStream* pStream = kStream.GetPtr();
		const auto meshPathAbs = (std::filesystem::current_path() / "Data/Meshes" / path).string();
		if (!pStream->Load2(meshPathAbs.c_str())) {
			log("Failed to load mesh: %s", path.c_str());
			log("Stream error: %s", pStream->m_acLastErrorMessage);
			return nullptr;
		}

		const NiObject* pObject = pStream->GetObjectAt(0);
		if (!pObject) {
			log("Failed to load root node of mesh: %s", path.c_str());
			return nullptr;
		}

		const auto node = pObject->NiDynamicCast<NiNode>();
		const auto fixedStr = NiFixedString(nodeName.c_str());
		const auto targetNode = node->GetObjectByName(fixedStr);
		if (!targetNode) {
			log("Node does not exist");
			return nullptr;
		}

		const auto geom = targetNode->NiDynamicCast<NiGeometry>();
		if (!geom) {
			log("Node is not geometry");
			return nullptr;
		}

		if (!geom->m_spModelData->m_pkColor) {
			log("Node does not have vertex colors");
			return nullptr;
		}

		constexpr auto ucKeepFlags =
			NiGeometryData::KEEP_XYZ |
			NiGeometryData::KEEP_NORM |
			NiGeometryData::KEEP_INDICES |
			NiGeometryData::KEEP_BONEDATA |
			NiGeometryData::KEEP_COLOR;

		geom->m_spModelData->m_ucKeepFlags = ucKeepFlags;
		geom->m_spModelData->m_usDirtyFlags = NiGeometryData::Consistency::MUTABLE;

		cache[key] = geom->m_spModelData->Clone()->NiDynamicCast<NiGeometryData>();
		return geom->m_spModelData;
	}

	static void ModProps(const NiAVObjectPtr& node, const char* modelPath = nullptr) {
		BEGIN_SCOPE();
		log("Processing node %s", node->GetName());
		bool bAddController = false;

		if (const auto niNode = node->NiDynamicCast<NiNode>()) {
			BEGIN_SCOPE();
			log("Is NiNode (%d children)", niNode->m_kChildren.m_usSize);

			for (uint32_t i = 0; i < niNode->m_kChildren.m_usSize; i++) {
				if (auto child = niNode->m_kChildren[i]) {
					ModProps(child, modelPath);
				}
			}
		}

		if (const auto geom = node->NiDynamicCast<NiGeometry>()) {
			BEGIN_SCOPE();
			if (matProps.contains(node->GetName()) && geom->GetMaterialProperty()) {
				log("Material properties enabled");
				bAddController = true;
			}
			else {
				BEGIN_SCOPE();
				log("Skipping material properties.. Not configured");
			}

			if (vertexColors.contains(node->GetName())) {
				BEGIN_SCOPE();
				if (!modelPath) {
					log("Node is configured to replace geometry data but no model path was provided.");
				}
				else {
					// Preload the vertex data
					if (const auto newGeomData = PreloadGeometryData(node->GetName(), modelPath)) {
						geom->SetModelData(newGeomData);

						log("Vertex colors enabled");
						bAddController = true;
					}
				}
			}
			else {
				BEGIN_SCOPE();
				log("Skipping vertex color flags.. Not configured");
			}
		}

		// if (const auto light = node->NiDynamicCast<NiLight>()) {
		// 	BEGIN_SCOPE();
		// 	log("NiLight");
		// 	bAddController = true;
		// }

		if (const auto psys = node->NiDynamicCast<NiParticleSystem>()) {
			BEGIN_SCOPE();
			log("NiParticleSystem");
			bAddController = true;
		}

		if (bAddController) {
			BEGIN_SCOPE();
			log("Adding NiHeatController");
			NiHeatController::CreateObject()->SetTarget(node);
		}
	}

	static void InjectMPS(const NiNodePtr& node) {
		BEGIN_SCOPE();
		log("Processing node: %s", node->GetName());

		if (auto* valueNode = node->NiDynamicCast<BSValueNode>()) {
			BEGIN_SCOPE();

			const auto addonNodeID = valueNode->iValue;
			const auto addonNode = TESDataHandler::GetSingleton()->GetAddonNode(addonNodeID);

			log("Addon node value: %d", addonNodeID);
			log("Addon node: 0x%p", addonNode);

			if (addonNode && addonNode->kData.ucFlags.GetBit(1)) {
				BEGIN_SCOPE();

				const auto manager = BSParticleSystemManager::GetInstance();
				const auto particleSystemIndex = addonNode->particleSystemID;
				log("Particle system ID: %d", particleSystemIndex);

				if (const auto mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>()) {
					BEGIN_SCOPE();
					log("MPS: 0x%p", mps);

					if (const auto newNode = mps->GetAt(0)->Clone()->NiDynamicCast<NiNode>()) {
						BEGIN_SCOPE();
						log("Grabbed main MPS node: %s", newNode->GetName());

						// Ensure that 3d offsets are correct
						newNode->m_kLocal = valueNode->m_kLocal;
						newNode->m_kWorld = valueNode->m_kWorld;

						// Copy value node children
						for (int i = 0; i < valueNode->m_kChildren.m_usSize; i++) {
							if (const auto child = valueNode->m_kChildren.m_pBase[i]) {
								BEGIN_SCOPE();
								log("Attaching value node child: %s", child->GetName());
								newNode->AttachChild(child->Clone()->NiDynamicCast<NiAVObject>(), true);
							}
						}

						// Copy value node controllers
						auto curController = valueNode->m_spControllers;
						while (curController) {
							BEGIN_SCOPE();
							log("Attaching value node controller: %s", curController->GetRTTIName());

							const auto newController = curController->Clone()->NiDynamicCast<NiTimeController>();
							newController->SetTarget(newNode);
							newController->SetActive(true);

							curController = curController->GetNext();
						}

						BEGIN_SCOPE();

						// Remove value node and add our new copy
						node->m_pkParent->AttachChild(newNode, false);
						node->m_pkParent->DetachChildAlt(valueNode);

						log("Attached new node");
					}
				}
			}
		}
		else {
			BEGIN_SCOPE();
			log("Node is not a BSValueNode. Skipping.");
		}

		// Process children
		BEGIN_SCOPE();
		log("Processing children (%d)", node->m_kChildren.m_usSize);
		for (int i = 0; i < node->m_kChildren.m_usSize; i++) {
			const auto child = node->m_kChildren[i];
			if (!child) {
				continue;
			}

			BEGIN_SCOPE();
			log("Child: %s", child->GetName());

			if (const auto childNode = child->NiDynamicCast<NiNode>()) {
				InjectMPS(childNode);
			}

			const auto psys = child->NiDynamicCast<NiParticleSystem>();
			if (!psys) {
				continue;
			}

			BEGIN_SCOPE();
			//psys->m_bWorldSpace = false;
			log("Is particle system");
			if (const auto bsp = psys->GetController<BSPSysMultiTargetEmitterCtlr>()) {
				BEGIN_SCOPE();
				log("Replacing BSPSysMultiTargetEmitterCtlr");

				NiCloningProcess cloner{};
				const NiPSysEmitterCtlrPtr newEmit = ThisStdCall<NiPSysEmitterCtlr*>(0xC1C570, bsp, &cloner);
				newEmit->SetTarget(child);

				// Should have a NiPSysUpdateCtlr next
				if (bsp->m_spNext) {
					bsp->m_spNext->SetTarget(child);
				}

				// Remove the BSPSysMultiTargetEmitterCtlr
				child->RemoveController(bsp);
			}
		}
	}

	static NiNode* __fastcall hkModelLoaderLoadFile(const uint8_t* model) {
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		const char* path = *reinterpret_cast<char**>(ebp + 0x8);

		if (path && (definedModels.contains(path) || strstr(path, "Flash"))) {
			auto* node = ThisStdCall<NiNode*>(0x43B230, model);

			std::println("Modifying model '{}'", path);

			InjectMPS(node);
			ModProps(node, path);

			return node;
		}

		return ThisStdCall<NiNode*>(0x43B230, model);
	}

	static void __fastcall hk_Model_Model(const uint8_t* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV) {
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);
		hkModelLoaderLoadFile(thisPtr);
	}

	inline void __fastcall hkMuzzleFlash_Enable(uint8_t* flash) {
		// MuzzleFlash::spLight
		(*reinterpret_cast<NiPointLight**>(flash + 0x10))->SetDiffuseColor(rgbShift(GetTickCount()));

		// MuzzleFlash::Enable
		ThisStdCall(0x9BB690, flash);
	}

	inline void Hook() {
		WriteRelCall(0x43AB4C, &hk_Model_Model);
		WriteRelCall(0x9BB7CD, &hkMuzzleFlash_Enable);
	}

	inline void PostLoad() {
		// Add any extra models
		for (const auto& elem : extraModels) {
			definedModels.insert(elem);
		}

		// Lookup specified forms by editor id, requires JG
		for (const auto& formName : editorIds) {
			const auto found = CdeclCall<TESForm*>(0x483A00, formName.c_str());
			if (!found || !found->IsObject()) {
				continue;
			}

			const auto obj = reinterpret_cast<TESObject*>(found);
			if (const auto fName = obj->GetModelFileName()) {
				definedModels.insert(fName);
				log("Added model %s", fName);
			}
		}
	}
}
