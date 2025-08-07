#include  "OverchargeHooks.hpp"

namespace Overcharge
{
	std::unordered_map<UInt64, std::shared_ptr<HeatData>>		activeWeapons;
	std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>	activeInstances;
	std::unordered_set<BSPSysSimpleColorModifier*>				colorModifiers; 

	void WeaponCooldown()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;

		for (auto it = activeWeapons.begin(); it != activeWeapons.end();)
		{
			auto& instance = it->second;
			if (!instance || (instance->state.fHeatVal -= frameTime * instance->data->fCooldownPerSecond) <= 0.0f)
			{
				it = activeWeapons.erase(it);
				continue;
			}

			instance->fx.currCol = SmoothColorShift(instance->state.fHeatVal, instance->data->iMinColor, instance->data->iMaxColor);
			for (const auto& node : instance->fx.targetBlocks)
			{
				if (node && node.m_pObject)
					SetEmissiveColor(node.m_pObject, instance->fx.currCol, instance->fx.matProp);
			}

			++it;
		}
	}

	inline void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, Actor* rActor)
	{
		const NiNodePtr sourceNode = rActor->Get3D();

		TESAmmo* equippedAmmo = rWeap->GetEquippedAmmo(rActor);
		if (!equippedAmmo || g_OCSettings.iEnableGameplayEffects < 1)
		{
			ThisStdCall(0x523150, rWeap, rActor);
			return;
		}

		UInt64 configKey = MakeHashKey(rWeap->uiFormID, equippedAmmo->uiFormID);
		const auto dataIt = weaponDataMap.find(configKey);
		if (dataIt == weaponDataMap.end())
		{
			ThisStdCall(0x523150, rWeap, rActor);
			return;
		}

		auto heat = GetOrCreateHeat(rActor->uiFormID, rWeap->uiFormID, sourceNode.m_pObject, dataIt->second);
		heat->state.HeatOnFire();
		if (heat->data != &dataIt->second)
		{
			heat->data = &dataIt->second;
		}
		ThisStdCall(0x523150, rWeap, rActor);
	}

	inline void __fastcall MuzzleFlashEnable(MuzzleFlash* flash)
	{
		const NiNodePtr muzzleNode = flash->spNode;
		UInt32 sourceID = flash->pSourceActor->uiFormID;
		UInt32 weapID = flash->pSourceWeapon->uiFormID;

		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			heat->fx.currCol = SmoothColorShift(heat->state.fHeatVal, heat->data->iMinColor, heat->data->iMaxColor);
			flash->spLight->SetDiffuseColor(heat->fx.currCol);
			TraverseNiNode<NiGeometry>(muzzleNode, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});
		}
		ThisStdCall(0x9BB690, flash);
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj)
	{
		const NiNodePtr projNode = proj->Get3D();
		UInt32 sourceID = proj->pSourceRef->uiFormID;
		UInt32 weapID = proj->pSourceWeapon->uiFormID;

		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			heat->fx.currCol = SmoothColorShift(heat->state.fHeatVal, heat->data->iMinColor, heat->data->iMaxColor);
			TraverseNiNode<BSValueNode>(projNode, [&heat](BSValueNode* valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(projNode, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});
			TraverseNiNode<NiParticleSystem>(projNode, [&heat](NiParticleSystem* psys) {
				activeInstances[psys] = heat;
				});
		}
		ThisStdCall(0x9A52F0, a1, proj);
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B8);
		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);
		if (!impact || !impact->spParticleObject || !pProjectile) return impact;

		const NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		UInt32 sourceID = pProjectile->pSourceRef->uiFormID;
		UInt32 weapID = pProjectile->pSourceWeapon->uiFormID;

		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			TraverseNiNode<BSValueNode>(impactNode, [&heat](BSValueNode* valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(impactNode, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});
		}
		return impact;
	}

	BSTempEffectParticle* __cdecl ImpactActorWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B0);
		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);
		if (!impact || !impact->spParticleObject || !pProjectile) return impact;

		const NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		UInt32 sourceID = pProjectile->pSourceRef->uiFormID;
		UInt32 weapID = pProjectile->pSourceWeapon->uiFormID;

		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			TraverseNiNode<BSValueNode>(impactNode, [&heat](BSValueNode* valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(impactNode, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});
		}
		return impact;
	}

	MagicShaderHitEffect* __fastcall MSHEInit(MagicShaderHitEffect* thisPtr, void* edx, TESObjectREFR* target, TESEffectShader* a3, float duration)
	{
		Actor* targetActor = reinterpret_cast<Actor*>(target);
		if (!targetActor) return ThisStdCall<MagicShaderHitEffect*>(0x81F580, thisPtr, target, a3, duration);

		Actor* killer = targetActor->pKiller;
		UInt32 killerID = killer->uiFormID;
		UInt32 killerWeapID = killer->GetCurrentWeaponID();

		if (auto heat = GetActiveHeat(killerID, killerWeapID))
		{
			UInt32 col = RGBtoUInt32(heat->fx.currCol);
			a3->Data.colorKey1RGB = col;
			a3->Data.colorKey2RGB = col;
			a3->Data.colorKey3RGB = col;
			a3->Data.uiEdgeColor = col;
			a3->Data.edgeColorRGB = col;
			a3->Data.uiFillColor1 = col;
		}
		return ThisStdCall<MagicShaderHitEffect*>(0x81F580, thisPtr, target, a3, duration);
	}

	TESObjectREFR* __fastcall CreateRefAtLocation(TESDataHandler* thisPtr, void* edx, TESBoundObject* pObject, NiPoint3* apLocation, NiPoint3* apDirection, TESObjectCELL* pInterior, TESWorldSpace* pWorld, TESObjectREFR* pReference, BGSPrimitive* pAddPrimitive, void* pAdditionalData)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Actor* targetActor = *reinterpret_cast<Actor**>(ebp + 0x20);
		TESObjectREFR* expl = ThisStdCall<TESObjectREFR*>(0x4698A0, thisPtr, pObject, apLocation, apDirection, pInterior, pWorld, pReference, pAddPrimitive, pAdditionalData);

		Actor* killer = targetActor->pKiller;
		UInt32 killerID = killer->uiFormID;
		UInt32 killerWeapID = killer->GetCurrentWeaponID();

		if (auto heat = GetActiveHeat(killerID, killerWeapID))
		{
			NiNodePtr node = pObject->LoadGraphics(expl);

			TraverseNiNode<NiGeometry>(node, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});

		}
		return expl;
	}

	__declspec(naked) void __fastcall ComputeInitPosVelocity(NiPSysVolumeEmitter* thisPtr, void* edx, NiPoint3* arg0, NiPoint3* arg4)
	{
		__asm
		{
			sub		esp, 0xD0
			push	0xC31CF6
			retn
		}
	}

	void __fastcall VertexColorModifier(NiPSysVolumeEmitter* thisPtr, void* edx, NiPoint3* arg0, NiPoint3* arg4)
	{
		if (!thisPtr || !thisPtr->m_pkEmitterObj || !thisPtr->m_pkTarget)
		{
			ComputeInitPosVelocity(thisPtr, edx, arg0, arg4);
			return;
		}
		if (auto it = activeInstances.find(thisPtr->m_pkEmitterObj); it != activeInstances.end())
		{
			NiAlphaPropertyPtr alpha = thisPtr->m_pkTarget->GetAlphaProperty();
			if (!alpha || !alpha->IsDstBlendMode(NiAlphaProperty::AlphaFunction::ALPHA_ONE))
			{
				activeInstances.erase(it);
			}
			else
			{
				const auto& heatInfo = it->second;
				thisPtr->m_kInitialColor = heatInfo->fx.currCol;
				for (const auto& modifier : thisPtr->m_pkTarget->m_kModifierList)
				{
					if (const auto scm = modifier->NiDynamicCast<BSPSysSimpleColorModifier>())
					{
						colorModifiers.emplace(scm);
					}
				}
				if (auto matProp = thisPtr->m_pkTarget->GetMaterialProperty())
				{
					matProp->m_emit = NiColor(1, 1, 1);
				}
			}
		}
		ComputeInitPosVelocity(thisPtr, edx, arg0, arg4);
	}

	__declspec(naked) void __fastcall OriginalColorModifierUpdate(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData)
	{
		__asm
		{
			push	ebp
			mov		ebp, esp
			sub		esp, 0x34
			push	0xC602E6
			retn
		}
	}

	inline void __fastcall ColorModifierUpdate(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData) 
	{
		if (!apData->m_pkColor || !colorModifiers.contains(apThis) || g_OCSettings.iEnableVisualEffects < 1)
		{
			OriginalColorModifierUpdate(apThis, edx, afTime, apData);
			return;
		}
		for (int i = 0; i < apData->m_usActiveVertices; ++i) 
		{
			NiParticleInfo* pInfo = &apData->m_pkParticleInfo[i];
			float lifePercent = pInfo->m_fAge / pInfo->m_fLifeSpan;

			if (apThis->fFadeOut != 0.0f && lifePercent > apThis->fFadeOut) {
				float fadeOutPercent = (lifePercent - apThis->fFadeOut) / (1.0f - apThis->fFadeOut);
				apData->m_pkColor[i].a = (apThis->kColor3.a - apThis->kColor2.a) * fadeOutPercent + apThis->kColor2.a;
			}
			else if (apThis->fFadeIn == 0.0f || lifePercent >= apThis->fFadeIn) {
				apData->m_pkColor[i].a = apThis->kColor2.a;
			}
			else {
				float fadeInPercent = lifePercent / apThis->fFadeIn;
				apData->m_pkColor[i].a = (apThis->kColor2.a - apThis->kColor1.a) * fadeInPercent + apThis->kColor1.a;
			}
		}
		return;
	}

	void __stdcall ClearTrackedEmitters(NiAVObject* toDelete)
	{
		activeInstances.erase(toDelete);

		std::erase_if(colorModifiers, [](BSPSysSimpleColorModifier* mod) {
			return mod == nullptr;
			});
	}

	__declspec(naked) void ClearUnrefEmittersHook()
	{
		static const UInt32 returnAddr = 0xC5E169;

		__asm
		{
			mov        eax, [ebp - 0x20]
			push	   eax
			call	   ClearTrackedEmitters

			mov        ecx, [ebp - 0x20]
			mov        edx, [ecx]
			jmp        returnAddr
		}
	}

	void __fastcall SetAttackSpeedHook(Animation* thisPtr, void* edx, float animSpeed) 
	{
		UInt32 sourceRef = thisPtr->pActor->uiFormID;
		UInt32 sourceWeap = thisPtr->pActor->GetCurrentWeaponID();
		if (auto heat = GetActiveHeat(sourceRef, sourceWeap))
		{
			float& currInfo = heat->state.fHeatVal;
			float heatRatio = currInfo / 100.0f;
			heatRatio = std::clamp(heatRatio, 0.0f, 1.0f);
			animSpeed *= (1.0f + heatRatio);
		}
		ThisStdCall(0x4C0C90, thisPtr, animSpeed);
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;			//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;			//0x447168 Checks loaded NIF file
		UInt32 createProjectile = 0x9BD518;  
		UInt32 spawnImpact = 0x9C2058;			//Projectile::SpawnCollisionEffects @ Projectile::ProcessImpacts 
		UInt32 spawnImpactEffects = 0x9C2AC3;  
		UInt32 spawnActorImpactEffects = 0x88F245;
		UInt32 muzzleFlashEnable = 0x9BB7CD; 
		UInt32 initParticle = 0xC2237A;
		UInt32 colorParticle = 0xC220E5;
		UInt32 GetAttackSpeedMult = 0x645D73;
		UInt32 GetWeaponAnimMult = 0x9CBE83;
		UInt32 SetAttackSpeed = 0x894337;
		UInt32 MPSAddon = 0x9BE07A;
		UInt32 initialPosVelocity = 0xC31CF0;
		UInt32 updateSCM = 0xC602E0;
		UInt32 clearUnrefEmitters = 0xC5E164;
		UInt32 colorModUpdate = 0xC602E0;
		UInt32 impactBodyPart = 0x8B5D9B;
		UInt32 DismemberBodyPart = 0x8B5135; 
		UInt32 DisintegrateEffectShader = 0x8214E8;
		UInt32 StartParticleShader = 0x82021B;
		UInt32 SetupParticleShader = 0x5077ED;
		UInt32 MagicShaderHitEffectApply = 0x81FC70;
		UInt32 MagicShaderAddTempEffect = 0x5D1CD1;
		UInt32 InitMagicShaderCmd = 0x5D1C90;
		UInt32 AshPilePersists = 0x5DBD8E;
		UInt32 GetAshXData = 0x5DBD9D;
		UInt32 SetAshXData = 0x5DBDBA;
		UInt32 CreateRefAtLoc = 0x5DBD56;

		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(spawnActorImpactEffects, &ImpactActorWrapper);
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
		WriteRelJump(initialPosVelocity, &VertexColorModifier);
		WriteRelJump(clearUnrefEmitters, &ClearUnrefEmittersHook);
		WriteRelJump(colorModUpdate, &ColorModifierUpdate);
		WriteRelCall(SetAttackSpeed, &SetAttackSpeedHook);
		WriteRelCall(InitMagicShaderCmd, &MSHEInit);
		WriteRelCall(CreateRefAtLoc, &CreateRefAtLocation);
	}
}