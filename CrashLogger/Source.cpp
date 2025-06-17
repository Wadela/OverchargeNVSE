#pragma once
#include "MainHeader.hpp"
#include "Overcharge.hpp"

#include "NiRTTI.hpp"
#include "NiObject.hpp"
#include "NiParticleSystem.hpp"
#include "NiAVObject.hpp"
#include <BSTempEffectParticle.hpp>
#include <BSValueNode.hpp>
#include <BSParticleSystemManager.h>

#include <filesystem>
#include <numbers>
#include <print>
#include <set>
#include <unordered_map>

#include "BGSAddonNode.hpp"
#include "BSMasterParticleSystem.hpp"
#include "BSPSysMultiTargetEmitterCtlr.hpp"
#include "BSStream.hpp"
#include "Defines.h"
#include "NiCloningProcess.hpp"
#include "NiGeometryData.hpp"
#include "NiNode.hpp"
#include "SafeWrite.hpp"
#include "TESMain.hpp"
#include "NiGeometry.hpp"
#include "NiStream.hpp"
#include "NiLight.hpp"
#include "NiPointLight.hpp"
#include "TESDataHandler.hpp"
#include <BSPSysSimpleColorModifier.hpp>
#include "NiUpdateData.hpp"
#include "TimeGlobal.hpp"
#include <TESObjectWEAP.hpp>
#include "PluginAPI.hpp"
#include "MuzzleFlash.hpp"
#include <NiBillboardNode.hpp>
#include "BGSImpactData.hpp" 
#include "TESModel.hpp"
#include <NifOverride.hpp>
#include "WeaponDataMap.hpp"
#include "NiPSysModifier.hpp"
#include "NiParticleInfo.hpp"
#include <NiPSysAgeDeathModifier.hpp>
#include <NiTransformInterpolator.hpp>
#include <NiPSysBoxEmitter.hpp>
//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	NiMaterialPropertyPtr g_customPlayerMatProp = NiMaterialProperty::CreateObject();

	std::vector<HeatInstance> activeWeapons;				//Vector containing all weapons that are currently heating up
	std::vector<Projectile*> activeProjectiles;
	std::unordered_map<UInt32, HeatInstance> weaponDataMap;

	std::vector<ParticleInstance> newActiveParticles;

	NiPoint3* GetRoundedPosition(NiPoint3* pos)
	{
		float roundX = std::round(pos->x);
		float roundY = std::round(pos->y);
		float roundZ = std::round(pos->z);

		return new NiPoint3(roundX, roundY, roundZ);
	}

	inline bool AreFloatsNearlyEqual(float a, float b, float epsilon = 1e-5f)
	{
		return fabsf(a - b) < epsilon;
	}

	inline bool AreNiPoint3NearlyEqual(const NiPoint3& a, const NiPoint3& b, float epsilon = 1e-5f)
	{
		return AreFloatsNearlyEqual(a.x, b.x, epsilon) &&
			AreFloatsNearlyEqual(a.y, b.y, epsilon) &&
			AreFloatsNearlyEqual(a.z, b.z, epsilon);
	}

	inline bool AreNiMatrix3NearlyEqual(const NiMatrix3& a, const NiMatrix3& b, float epsilon = 1e-5f)
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				if (!AreFloatsNearlyEqual(a.m_pEntry[row][col], b.m_pEntry[row][col], epsilon))
					return false;
			}
		}
		return true;
	}

	inline bool AreNiTransformsNearlyEqual(const NiTransform& a, const NiTransform& b, float epsilon = 1e-5f)
	{
		return AreFloatsNearlyEqual(a.m_fScale, b.m_fScale, epsilon) &&
			AreNiPoint3NearlyEqual(a.m_Translate, b.m_Translate, epsilon) &&
			AreNiMatrix3NearlyEqual(a.m_Rotate, b.m_Rotate, epsilon);
	}

	static void SetEmissiveByName(NiAVObjectPtr obj, NiMaterialProperty* matProp, NiColor& blendedColor, const char* blockName = nullptr)
	{
		if (!obj) return;
		NiAVObjectPtr target = blockName ? obj->GetObjectByName(blockName) : obj.m_pObject;

		if (!target) return;
		if (NiGeometry* geom = target->NiDynamicCast<NiGeometry>())
		{
			if (!geom->m_kProperties.m_spMaterialProperty) return;
			geom->m_kProperties.m_spMaterialProperty = matProp;
			geom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor;
		}
	}

	static void SetEmissiveColor(NiAVObjectPtr obj, const NiColor& blendedColor)
	{
		NiGeometryPtr geom = obj ? obj->NiDynamicCast<NiGeometry>() : nullptr;

		if (!geom) return;
		NiMaterialPropertyPtr matProp = geom->m_kProperties.m_spMaterialProperty;

		if (!matProp) return;
		matProp->m_emit = blendedColor;
	}

	static void SetEmissiveClone(NiAVObjectPtr obj, const NiColor& blendedColor)
	{
		NiGeometryPtr geom = obj ? obj->NiDynamicCast<NiGeometry>() : nullptr;

		if (!geom) return;
		NiMaterialPropertyPtr originalMat = geom->m_kProperties.m_spMaterialProperty;

		if (!originalMat) return;
		//NiMaterialPropertyPtr clonedMat = originalMat->Clone()->NiDynamicCast<NiMaterialProperty>();
		NiMaterialPropertyPtr clonedMat = NiMaterialProperty::CreateObject();

		if (!clonedMat) return;
		clonedMat->m_emit = blendedColor;
		geom->m_kProperties.m_spMaterialProperty = clonedMat;
	}

	//Update Child Particles to all be prepared for emissive color control
	static void ChangeParticleColor(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
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
						SetEmissiveColor(mps->kChildParticles[i], blendedColor);
					}
				}
			}
		}
	}


	//NOTE TO SELF: YOU NEED TO MAKE THIS FUNCTION TRAVERSE THE MPS AND GET ALL CHILDPARTICLESYSTEMS OTHERWISE IT WILL ONLY GRAB THE FIRST ONE AS YOU SAW WITH THE IMPACT. YOU NEED TO MAKE IT LOOK SIMILAR TO THE FUNCTION IN NIFOVERRIDE.


	//Update Child Particles to all be prepared for emissive color control
	static void CloneParticle1(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->uiIndex)
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					NiNodePtr mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>();
					if (!mpsNode) return; 
					if (NiNode* newNode = mpsNode->Clone()->NiDynamicCast<NiNode>())
					{
						std::pair<NiNode*, NiNode*> activeNodes = { newNode, valueNode };

						for (int i = 0; i < newNode->m_kChildren.m_usSize; i++)
						{
							if (const auto& child2 = newNode->m_kChildren.m_pBase[i])
							{
								if (NiParticleSystem* psys = child2->NiDynamicCast<NiParticleSystem>())
								{
									if (auto bsp = psys->GetController<BSPSysMultiTargetEmitterCtlr>())
									{
										NiCloningProcess cloner{};
										if (NiPSysEmitterCtlrPtr newEmit = ThisStdCall<NiPSysEmitterCtlr*>(0xC1C570, bsp, &cloner))
										{
											newEmit->SetTarget(psys);
											psys->RemoveController(bsp);
											psys->m_fLastTime = 0;
										}
									}
									SetEmissiveClone(psys, blendedColor);
									newActiveParticles.emplace_back(ParticleInstance(psys, activeNodes));
									mps->FindRootNode()->AttachChild(newNode, false);
								}
							}
						}
						valueNode->SetAppCulled(true);
					}
				}
			}
		}
	}

	//Update Child Particles to all be prepared for emissive color control
	static void CloneParticle3(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->uiIndex)
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					NiNodePtr mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>();
					if (!mpsNode) return;
					if (NiNode* newNode = mpsNode->Clone()->NiDynamicCast<NiNode>())
					{
						std::pair<NiNode*, NiNode*> activeNodes = { newNode, valueNode };

						for (int i = 0; i < newNode->m_kChildren.m_usSize; i++)
						{
							if (const auto& child2 = newNode->m_kChildren.m_pBase[i])
							{
								if (NiParticleSystem* psys = child2->NiDynamicCast<NiParticleSystem>())
								{
									if (auto bsp = psys->GetController<BSPSysMultiTargetEmitterCtlr>())
									{
										NiCloningProcess cloner{};
										if (NiPSysEmitterCtlrPtr newEmit = ThisStdCall<NiPSysEmitterCtlr*>(0xC1C570, bsp, &cloner))
										{
											newEmit->m_fStartTime = 0;
											newEmit->m_fLastTime = 0;
											newEmit->m_fScaledTime = 0;
											newEmit->SetTarget(psys);
											psys->RemoveController(bsp);
											psys->m_fLastTime = 0;
										}
									}
									SetEmissiveClone(psys, blendedColor);
									newActiveParticles.emplace_back(ParticleInstance(psys, activeNodes));
									mps->FindRootNode()->AttachChild(newNode, false);
								}
								if (NiNode* emitterTest = child2->NiDynamicCast<NiNode>())
								{
									std::pair<NiNode*, NiNode*> activeNodes1 = { emitterTest, valueNode };
									for (int i = 0; i < emitterTest->m_kChildren.m_usSize; i++)
									{
										if (const auto& child3 = emitterTest->m_kChildren.m_pBase[i])
										{
											if (NiParticleSystem* psys1 = child3->NiDynamicCast<NiParticleSystem>())
											{
												if (auto bsp1 = psys1->GetController<BSPSysMultiTargetEmitterCtlr>())
												{
													NiCloningProcess cloner1{};
													if (NiPSysEmitterCtlrPtr newEmit1 = ThisStdCall<NiPSysEmitterCtlr*>(0xC1C570, bsp1, &cloner1))
													{
														newEmit1->SetTarget(psys1);
														psys1->RemoveController(bsp1);
														psys1->m_fLastTime = 0;
													}
												}
												SetEmissiveClone(psys1, blendedColor);
												newActiveParticles.emplace_back(ParticleInstance(psys1, activeNodes1));
												mps->FindRootNode()->AttachChild(newNode, false);
											}
										}
									}
								}
							}
						}
						valueNode->SetAppCulled(true);
					}
				}
			}
		}
	}

	//Update Child Particles to all be prepared for emissive color control
	static void CloneParticle(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->uiIndex)
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					NiNodePtr mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>();
					if (!mpsNode) return;
					if (NiNode* newNode = mpsNode->Clone()->NiDynamicCast<NiNode>())
					{
						ThisCall(0x546780, newNode, true); //NiAVObject::SetAlwaysDraw();
						//newNode->CreateWorldBoundIfMissing();
						std::pair<NiNode*, NiNode*> activeNodes = { newNode, valueNode };

						for (int i = 0; i < newNode->m_kChildren.m_usSize; i++)
						{
							if (const auto& child2 = newNode->m_kChildren.m_pBase[i])
							{
								if (NiParticleSystem* psys = child2->NiDynamicCast<NiParticleSystem>())
								{
									if (auto bsp = psys->GetController<BSPSysMultiTargetEmitterCtlr>())
									{
										NiCloningProcess cloner{};
										if (NiPSysEmitterCtlrPtr newEmit = ThisStdCall<NiPSysEmitterCtlr*>(0xC1C570, bsp, &cloner))
										{
											newEmit->SetTarget(psys);
											psys->RemoveController(bsp);
											//psys->m_fLastTime = 0;
										}
									}
									//going to change this whole function to look more like this. So it will go through child particles then look for the emitters similar to how this segment does it. 
									if (auto* sysData = psys->m_spModelData->NiDynamicCast<NiPSysData>())
									{
										for (NiPSysModifier* mod : psys->m_kModifierList)
										{
											if (NiPSysBoxEmitter* emitter = mod->NiDynamicCast<NiPSysBoxEmitter>())
											{
												if (NiNode* emitNode = emitter->m_pkEmitterObj->NiDynamicCast<NiNode>())
												{
													std::pair<NiNode*, NiNode*> activeNodes1 = { emitNode, valueNode };
													newActiveParticles.emplace_back(ParticleInstance(psys, activeNodes1));
												}
											}
										}
									}
									SetEmissiveClone(psys, blendedColor);
									newActiveParticles.emplace_back(ParticleInstance(psys, activeNodes));
									mps->FindRootNode()->AttachChild(newNode, false);
								}
							}
						}
						valueNode->SetAppCulled(true);
					}
				}
			}
		}
	}

	static void ProcessNiNode2(const NiNode* obj, NiColor& blendedColor)
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
					SetEmissiveColor(childPsys, blendedColor);
				}
				else if (child->IsNiType<BSValueNode>())
				{
					BSValueNode* childValNode = static_cast<BSValueNode*>(child);
					CloneParticle(childValNode, blendedColor);
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessNiNode2(childNode, blendedColor);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveColor(childGeom, blendedColor); 
				} 
			}
		}
	}

	static void ProcessNiNodeTest(const NiNode* obj, NiColor& blendedColor)
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
					SetEmissiveClone(childPsys, blendedColor);
				}
				else if (child->IsNiType<BSValueNode>())
				{
					BSValueNode* childValNode = static_cast<BSValueNode*>(child);
					ChangeParticleColor(childValNode, blendedColor);
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessNiNodeTest(childNode, blendedColor);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveClone(childGeom, blendedColor);
				}
			}
		}
	}


	void WeaponCooldown()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;

		activeWeapons.erase(
			std::remove_if(activeWeapons.begin(), activeWeapons.end(),
				[frameTime](Overcharge::HeatInstance& instance)
				{
					instance.heat.heatVal -= frameTime * instance.heat.cooldownRate;
					if (instance.heat.heatVal <= instance.heat.baseHeatVal)
						return true;

					instance.fx.currCol = instance.fx.SmoothShift(instance.heat.heatVal, instance.heat.maxHeat);
					for (const auto& node : instance.targetBlocks)
					{
						if (node && node.m_pObject)
						{
							SetEmissiveColor(node.m_pObject, instance.fx.currCol);
						}
					}
					return false;
				}),
			activeWeapons.end());
	}

	void ParticleUpdater()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;

		newActiveParticles.erase(
			std::remove_if(newActiveParticles.begin(), newActiveParticles.end(),
				[&](ParticleInstance& instance)
				{
					NiParticleSystem* psys = instance.particle;

					//psys->m_fLastTime += frameTime;

					if (!psys || !psys->m_spControllers) return true;

					const float hi = psys->m_spControllers->m_fHiKeyTime;
					const float lingerThreshold = hi + 4.0f ; 
					const bool inLingerWindow = psys->m_fLastTime <= hi;

					//if (inLingerWindow ) //&& !AreNiTransformsNearlyEqual(instance.nodePair.first->m_kWorld, instance.nodePair.second->m_kWorld)
					//{
					//instance.nodePair.first->m_kLocal = instance.nodePair.second->m_kLocal;
						NiUpdateData updateData{ frameTime, 1, 0 };
						instance.nodePair.first->m_pWorldBound = instance.nodePair.second->m_pWorldBound;
						instance.nodePair.first->m_kLocal = instance.nodePair.second->m_kWorld;
						instance.nodePair.first->Update(updateData);
						//instance.nodePair.first->m_kWorld = instance.nodePair.second->m_kWorld;
					//}

					if (auto* sysData = psys->m_spModelData->NiDynamicCast<NiPSysData>())
					{
						for (NiPSysModifier* mod : psys->m_kModifierList)
						{
							//if (mod) mod->Update(psys->m_fLastTime, sysData);
						}
					}

					return false;
				}),
			newActiveParticles.end());
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj)
	{
		NiNode* projNode = proj->Get3D();
		UInt32 sourceRef = proj->pSourceRef->uiFormID;
		UInt32 sourceWeap = proj->pSourceWeapon->uiFormID;

		activeProjectiles.emplace_back(proj);
		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap) == true)
			{
				ProcessNiNode2(projNode, it.fx.currCol);
			}
		}
		ThisStdCall(0x9A52F0, a1, proj);
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		BSTempEffectParticle* impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);
		NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		NiPoint3 roundedPos = GetRoundedPosition(&impactPos);

		activeProjectiles.erase(
			std::remove_if(activeProjectiles.begin(), activeProjectiles.end(),
				[&](Projectile* it)
				{
					if (it->bHasImpacted || it->IsDestroyed() || it->IsDeleted()) return true;
					if (it->pSourceRef && it->pSourceWeapon)
					{
						UInt32 sourceRef = it->pSourceRef->uiFormID;
						UInt32 sourceWeap = it->pSourceWeapon->uiFormID;

						for (auto& weaponIt : activeWeapons)
						{
							if (weaponIt.CompareForms(sourceRef, sourceWeap))
							{
								//ProcessNiNode2(impactNode, weaponIt.fx.currCol);
								//ProcessNiNodeTest(impactNode, weaponIt.fx.currCol);
							}
						}
					}
					return roundedPos == GetRoundedPosition(&it->kPosition);
				}),
			activeProjectiles.end());

		return impact;
	}

	inline void __fastcall MuzzleFlashEnable(MuzzleFlash* muzzleFlash)
	{
		NiNodePtr muzzleNode = muzzleFlash->spNode;
		UInt32 sourceRef = muzzleFlash->pSourceActor->uiFormID;
		UInt32 sourceWeap = muzzleFlash->pSourceWeapon->uiFormID;

		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap))
			{
				SetEmissiveClone(muzzleNode.m_pObject, it.fx.currCol);
				muzzleFlash->spLight->SetDiffuseColor(it.fx.currCol);
			}
		}
		ThisStdCall(0x9BB690, muzzleFlash);
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		UInt32 sourceRef = rActor->uiFormID;
		UInt32 sourceWeap = rWeap->uiFormID;
		NiNodePtr sourceNode = rActor->Get3D();
																			
		bool found = false;
		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap))
			{
				found = true;
				it.heat.HeatOnFire();
				break; 
			}
		}
		if (!found)
		{
			auto weaponIt = weaponDataMap.find(sourceWeap);
			if (weaponIt != weaponDataMap.end())
			{
				HeatInstance newHeat = MakeHeatFromTemplate(weaponIt->second, sourceNode.m_pObject, sourceRef, sourceWeap);
				newHeat.heat.HeatOnFire();
				newHeat.fx.currCol = newHeat.fx.SmoothShift(newHeat.heat.heatVal, newHeat.heat.maxHeat);

				for (NiAVObjectPtr it : newHeat.targetBlocks)
				{
					SetEmissiveClone(it, newHeat.fx.currCol);
				}
				activeWeapons.emplace_back(newHeat);
			}
		}

		// Calls original Actor::FireWeapon
		ThisStdCall(0x523150, rWeap, rActor);
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;			//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;			//0x447168 Checks loaded NIF file
		UInt32 createProjectile = 0x9BD518;  
		UInt32 spawnImpact = 0x9C2058;			//Projectile::SpawnCollisionEffects @ Projectile::ProcessImpacts 
		UInt32 spawnImpactEffects = 0x9C2AC3;  
		UInt32 muzzleFlashEnable = 0x9BB7CD; 

		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
	}

}