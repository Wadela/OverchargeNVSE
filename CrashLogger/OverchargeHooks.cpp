#include  "OverchargeHooks.hpp"
#include <BSShaderUtil.hpp>
#include "MTRenderingSystem.hpp"

namespace Overcharge
{
	std::unordered_map<UInt64, std::shared_ptr<HeatData>>		activeWeapons;
	std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>	activeInstances;
	std::unordered_set<BSPSysSimpleColorModifier*>				colorModifiers; 
	std::unordered_set<NiParticleSystem*>						worldSpaceParticles;

	bool UpdateWeaponInstance(std::shared_ptr<HeatData> instance, float frameTime, bool isPlayer = false)
	{
		if (!instance) return false;

		if (!(instance->state.uiOCEffect & (1 << 0)) && instance->state.fHeatVal > 100)
		{
			instance->state.uiOCEffect |= (1 << 0);
		}

		if (instance->state.uiOCEffect & (1 << 1) && 
			instance->state.fHeatVal < 99) {
			instance->state.fHeatVal += frameTime * instance->state.fHeatPerShot;
			instance->state.uiTicksPassed = 0;
		}

		if (instance->state.uiOCEffect & (1 << 2) && 
			instance->state.fHeatVal <= instance->state.uiOCEffectThreshold) {
			instance->state.fHeatVal += frameTime * instance->state.fHeatPerShot;
			instance->state.uiTicksPassed = 0;
		}

		instance->fx.currCol = SmoothColorShift(
			instance->state.fHeatVal,
			instance->data->iMinColor,
			instance->data->iMaxColor
		);

		for (const auto& node : instance->fx.targetBlocks) {
			if (node && node.m_pObject) {
				SetEmissiveColor(node.m_pObject, instance->fx.currCol, instance->fx.matProp);
			}
		}

		if (instance->state.uiTicksPassed < 255) ++instance->state.uiTicksPassed;
		float timePassed = instance->state.uiTicksPassed * frameTime;

		constexpr float cooldownDelaySeconds = 0.2f;
		if (timePassed >= cooldownDelaySeconds) {
			instance->state.fHeatVal -= frameTime * instance->state.fCooldownRate;
			if (instance->state.fHeatVal < 0.0f) instance->state.fHeatVal = 0.0f; 
		}

		constexpr float eraseDelaySeconds = 1.0f;
		if (instance->state.fHeatVal <= 0.0f && timePassed >= eraseDelaySeconds) {
			return false;
		}

		return true;
	}


	void WeaponCooldown()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;
		const UInt32 playerID = PlayerCharacter::GetSingleton()->uiFormID;

		for (auto it = activeWeapons.begin(); it != activeWeapons.end(); )
		{
			auto& instance = it->second;
			if (!instance) { ++it; continue; }

			UInt32 actorID = static_cast<UInt32>(it->first >> 32);
			if (actorID == playerID)
			{
				if (!UpdateWeaponInstance(instance, frameTime, true))
				{
					it = activeWeapons.erase(it);
					continue;
				}
			}
			++it;
		}

		for (auto it = activeWeapons.begin(); it != activeWeapons.end(); )
		{
			auto& instance = it->second;
			if (!instance) { ++it; continue; }

			UInt32 actorID = static_cast<UInt32>(it->first >> 32);
			if (actorID == playerID) { ++it; continue; } 

			if (!UpdateWeaponInstance(instance, frameTime, false))
			{
				it = activeWeapons.erase(it); 
				continue;
			}
			++it;
		}
	}

	bool __fastcall PlayFireAnimation(Actor* rActor, void* edx, UInt8 groupID)
	{
		if (!rActor || !rActor->pkBaseProcess) return ThisStdCall<bool>(0x893A40, rActor, groupID);

		UInt32 weapID = rActor->GetCurrentWeaponID();
		auto rWeap = (TESForm::GetByID(weapID));
		auto rWeapRef = reinterpret_cast<TESObjectWEAP*>(rWeap);
		const auto ogFireRate = rWeapRef->animAttackMult;

		if (rWeap)
		{
			auto heat = GetActiveHeat(rActor->uiFormID, rWeap->uiFormID);
			if (heat && rWeapRef)
			{
				float heatRatio = heat->state.fHeatVal / 100.0f;
				rWeapRef->animAttackMult = ScaleByPercentRange(rWeapRef->animAttackMult, heat->data->fMinFireRate, heat->data->fMaxFireRate, heatRatio);
				if (heat->state.uiOCEffect != 0)
				{
					rActor->pkBaseProcess->SetIsNextAttackLoopQueued(0);
					rActor->pkBaseProcess->SetForceFireWeapon(0);
					rActor->pkBaseProcess->SetIsFiringAutomaticWeapon(0);
					return ThisStdCall<bool>(0x893A40, rActor, 0xFF);
				}
			}
		}
		return ThisStdCall<bool>(0x893A40, rActor, groupID);
		rWeapRef->animAttackMult = ogFireRate; 
	}

	static inline void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, Actor* rActor)
	{
		const NiNodePtr sourceNode = rActor->Get3D();
		TESAmmo* equippedAmmo = rWeap->GetEquippedAmmo(rActor);
		if (!equippedAmmo)
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

		if (sourceNode)
		{
			auto smokeNode = sourceNode->GetObjectByName("OCSmoke");
			auto smokePsys = sourceNode->GetObjectByName("OCSmoke01");
			if (NiParticleSystemPtr smoke = smokePsys->NiDynamicCast<NiParticleSystem>())
			{
				worldSpaceParticles.emplace(smoke.m_pObject);
			}
			//auto smokeCtlr = smoke->GetControllers();
			//if (heat->state.fHeatVal >= 100)
			//{
				//smokeCtlr->SetActive(true);
				//auto pSceneGraph = TESMain::GetWorldRoot();
				//auto pCullingProcess = pSceneGraph->GetCullingProcess();
				//auto firstCam = TESMain::Get1stCamNode();
				//CdeclCall<void>(0xB6BEE0, firstCam, smokeNode, pCullingProcess);
			//}
			//else
				//smokeCtlr->SetActive(false);
		}

		//Backup Weapon Values - Backup needed since we're editing the baseform.
		const UInt8 ogAmmoUse = rWeap->ammoUse;
		const UInt8 ogProjectiles = rWeap->numProjectiles;
		const UInt16 ogDamage = rWeap->usAttackDamage;
		const UInt16 ogCritDamage = rWeap->criticalDamage;
		const float ogMinSpread = rWeap->minSpread;

		//Using ScaleByPercentRange() just in case a user has altered the base values at all.
		const HeatConfiguration* config = heat->data;
		const float hRatio = heat->state.fHeatVal / 100.0f;
		heat->state.uiAmmoUsed = ScaleByPercentRange(rWeap->ammoUse, config->iMinAmmoUsed, config->iMaxAmmoUsed, hRatio);
		heat->state.uiProjectiles = ScaleByPercentRange(rWeap->numProjectiles, config->iMinProjectiles, config->iMaxProjectiles, hRatio);
		heat->state.uiDamage = ScaleByPercentRange(rWeap->usAttackDamage, config->iMinDamage, config->iMaxDamage, hRatio);
		heat->state.uiCritDamage = ScaleByPercentRange(rWeap->criticalDamage, config->iMinCritDamage, config->iMaxCritDamage, hRatio);
		heat->state.fAccuracy = ScaleByPercentRange(rWeap->minSpread, config->fMinAccuracy, config->fMaxAccuracy, hRatio);

		rWeap->ammoUse = heat->state.uiAmmoUsed;
		rWeap->numProjectiles = heat->state.uiProjectiles;
		rWeap->usAttackDamage = heat->state.uiDamage;
		rWeap->criticalDamage = heat->state.uiCritDamage;
		rWeap->minSpread = heat->state.fAccuracy;

		ThisStdCall(0x523150, rWeap, rActor);

		//Restore Weapon Values - Need to restore so only the current weapon is altered.
		rWeap->ammoUse = ogAmmoUse;
		rWeap->numProjectiles = ogProjectiles;
		rWeap->usAttackDamage = ogDamage;
		rWeap->criticalDamage = ogCritDamage;
		rWeap->minSpread = ogMinSpread;
	}

	static inline void __fastcall MuzzleFlashEnable(MuzzleFlash* flash)
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

	static inline void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj)
	{
		const NiNodePtr projNode = proj->Get3D();
		UInt32 sourceID = proj->pSourceRef->uiFormID;
		UInt32 weapID = proj->pSourceWeapon->uiFormID;

		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			//Update Projectile Values - Don't need to backup because they're references.
			float heatRatio = heat->state.fHeatVal / 100.0f;
			projNode->m_kLocal.m_fScale = 
				InterpolateBasePercent(
					projNode->m_kLocal.m_fScale, 
					heat->data->iMinProjectileSizePercent, 
					heat->data->iMaxProjectileSizePercent, 
					heatRatio
				);
			heat->state.fProjectileSize = projNode->m_kLocal.m_fScale;

			proj->fSpeedMult =
				InterpolateBasePercent(
					proj->fSpeedMult,
					heat->data->iMinProjectileSpeedPercent,
					heat->data->iMaxProjectileSpeedPercent,
					heatRatio
				);
			heat->state.fProjectileSpeed = proj->fSpeedMult;

			//Update Current Color - Need to update early since it races WeaponCooldown() for current color.
			heat->fx.currCol = SmoothColorShift(heat->state.fHeatVal, heat->data->iMinColor, heat->data->iMaxColor);

			//Insert Value Nodes to activeInstances - BSValueNodes serve as emitter objects for their respective particles.
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

	static BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B8);
		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);

		if (!impact || !impact->spParticleObject || !pProjectile) return impact;

		const NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		if (!impactNode) return impact;

		UInt32 sourceID = pProjectile->pSourceRef->uiFormID;
		UInt32 weapID = pProjectile->pSourceWeapon->uiFormID;
		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			//Insert Value Nodes to activeInstances - BSValueNodes serve as emitter objects for their respective particles.
			TraverseNiNode<BSValueNode>(impactNode, [&heat](BSValueNode* valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(impactNode, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});
		}
		return impact;
	}

	static BSTempEffectParticle* __cdecl ImpactActorWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B0);
		Actor* actor = *reinterpret_cast<Actor**>(ebp - 0x18);
		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);

		if (!impact || !impact->spParticleObject || !pProjectile) return impact;

		const NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		if (!impactNode) return impact;

		UInt32 sourceID = pProjectile->pSourceRef->uiFormID;
		UInt32 weapID = pProjectile->pSourceWeapon->uiFormID;
		if (auto heat = GetActiveHeat(sourceID, weapID))		{
			if (actor && heat->data->iObjectEffectID && heat->state.fHeatVal >= heat->data->iObjectEffectThreshold)
			{
				TESForm* effect = TESForm::GetByID(heat->data->iObjectEffectID);
				actor->CastSpellImmediate(reinterpret_cast<MagicItemForm*>(effect), 0, actor, 1, 0);
			}

			//Insert Value Nodes to activeInstances - BSValueNodes serve as emitter objects for their respective particles.
			TraverseNiNode<BSValueNode>(impactNode, [&heat](BSValueNode* valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(impactNode, [&heat](NiGeometry* geom) {
				SetEmissiveColor(geom, heat->fx.currCol);
				});
		}
		return impact;
	}

	static MagicShaderHitEffect* __fastcall MSHEInit(MagicShaderHitEffect* thisPtr, void* edx, TESObjectREFR* target, TESEffectShader* a3, float duration)
	{
		MagicShaderHitEffect* mshe = ThisStdCall<MagicShaderHitEffect*>(0x81F580, thisPtr, target, a3, duration);
		Actor* targetActor = reinterpret_cast<Actor*>(target);
		if (!targetActor) return mshe;

		Actor* killer = targetActor->pKiller;
		if (!killer) return mshe;
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
		return mshe;
	}

	static TESObjectREFR* __fastcall CreateRefAtLocation(TESDataHandler* thisPtr, void* edx, TESBoundObject* pObject, NiPoint3* apLocation, NiPoint3* apDirection, TESObjectCELL* pInterior, TESWorldSpace* pWorld, TESObjectREFR* pReference, BGSPrimitive* pAddPrimitive, void* pAdditionalData)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Actor* targetActor = *reinterpret_cast<Actor**>(ebp + 0x20);
		TESObjectREFR* expl = ThisStdCall<TESObjectREFR*>(0x4698A0, thisPtr, pObject, apLocation, apDirection, pInterior, pWorld, pReference, pAddPrimitive, pAdditionalData);

		Actor* killer = targetActor->pKiller;
		if (!killer) return expl;
		UInt32 killerID = killer->uiFormID;
		UInt32 killerWeapID = killer->GetCurrentWeaponID();

		if (auto heat = GetActiveHeat(killerID, killerWeapID))
		{
			//LoadGraphics() is called earlier than the game usually would so we can control the pile's color.
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

	static void __fastcall VertexColorModifier(NiPSysVolumeEmitter* thisPtr, void* edx, NiPoint3* arg0, NiPoint3* arg4)
	{
		if (!thisPtr || !thisPtr->m_pkEmitterObj || !thisPtr->m_pkTarget)
		{
			ComputeInitPosVelocity(thisPtr, edx, arg0, arg4);
			return;
		}
		if (auto it = activeInstances.find(thisPtr->m_pkEmitterObj); it != activeInstances.end())
		{
			//Bright or more opaque particles should typically use a DstBlendMode of 'One' so checking it filters particles like smoke or dust.
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
					//Adding the active color modifier to target it for a later detour of its update.
					if (const auto scm = modifier->NiDynamicCast<BSPSysSimpleColorModifier>())
					{
						colorModifiers.emplace(scm);
					}
				}
				//Disabling emissive colors so the vertex colors control the appearance.
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

	static inline void __fastcall ColorModifierUpdate(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData) 
	{
		if (!apData->m_pkColor || !colorModifiers.contains(apThis) || g_OCSettings.iEnableVisualEffects < 1)
		{
			OriginalColorModifierUpdate(apThis, edx, afTime, apData);
			return;
		}
		//Detour Original Function - We ONLY update alphas otherwise the color modifier will reset vertex colors
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

	static void __stdcall ClearTrackedEmitters(NiAVObject* toDelete)
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

	__declspec(naked) void __fastcall UpdatePsysWorldData(NiParticleSystem* thisPtr, void* edx, NiUpdateData* apData)
	{
		__asm
		{
			mov     eax, [esp + 0x4]
			push    ebx
			push    0xC1ADD5
			retn
		}
	}

	static inline void __fastcall TogglePsysWorldUpdate(NiParticleSystem* thisPtr, void* edx, NiUpdateData* apData)
	{
		if (!thisPtr || !apData || !thisPtr->m_bWorldSpace || !worldSpaceParticles.contains(thisPtr))
		UpdatePsysWorldData(thisPtr, edx, apData);
		else
		{
			auto camera1st = PlayerCharacter::GetSingleton()->Get3D();
			ThisStdCall(0xA68C60, thisPtr, apData);
			memcpy(&thisPtr->m_kUnmodifiedWorld, &thisPtr->m_kWorld, sizeof(thisPtr->m_kUnmodifiedWorld));
			thisPtr->m_kWorld.m_Translate = camera1st->m_kWorld.m_Translate;
			memcpy(&thisPtr->m_kWorld.m_Rotate, &NiMatrix3::IDENTITY, sizeof(NiMatrix3));
		}
		return;
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

		UInt32 FireWeaponAnim = 0x949CF1;

		UInt32 Subtitle = 0x7052B8;
		UInt32 runDialogueResult = 0x83C88E;
		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(spawnActorImpactEffects, &ImpactActorWrapper);
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
		WriteRelJump(initialPosVelocity, &VertexColorModifier);
		WriteRelJump(clearUnrefEmitters, &ClearUnrefEmittersHook);
		WriteRelJump(colorModUpdate, &ColorModifierUpdate);
		WriteRelCall(InitMagicShaderCmd, &MSHEInit);
		WriteRelCall(CreateRefAtLoc, &CreateRefAtLocation);

		WriteRelCall(FireWeaponAnim, &PlayFireAnimation);

		//Testing
		WriteRelJump(0xC1ADD0, &TogglePsysWorldUpdate);
	}
}