#include "OverchargeHooks.hpp"

namespace Overcharge
{
	std::vector<std::shared_ptr<HeatData>>		playerOCWeapons;
	std::vector<std::shared_ptr<HeatData>>		activeOCWeapons;
	std::vector<NiParticleSystemPtr>			worldSpaceParticles;

	std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>				activeInstances;
	std::unordered_map<BSPSysSimpleColorModifier*, std::weak_ptr<HeatData>>	colorModifiers;

	std::unordered_map<BSPSysSimpleColorModifier*, ParticleInstance> colorMods;
	std::unordered_map<NiPSysData*, std::unordered_set<UInt16>>  activeParticles;

	BGSPerk* OCPerkOverclocker;
	BGSPerk* OCPerkVoltageRegulator;
	BGSPerk* OCPerkGalvanicRelativist;
	BGSPerk* OCPerkCircuitBender;
	BGSPerk* OCPerkCriticalMass;
	BGSPerk* OCPerkCoolantLeak;
	BGSPerk* OCPerkThermicInversion;

	NiAVObjectPtr OCTranslate;

	bool UpdateActiveWeapons(std::shared_ptr<HeatData> instance, float frameTime, bool isPlayer = false)
	{
		if (!instance || !instance->rActor || !instance->rWeap) return false;

		auto& st = instance->state;
		auto& fx = instance->fx;

		//If a weapon is inactive, it essentially is queued for cleanup.
		if (instance->rActor->GetCurrentWeapon() != instance->rWeap
			|| instance->rActor->IsDying()) st.bIsActive = false;

		//Refresh OCWeapons when it becomes active again to avoid issues with the game unloading anything important.
		if (!st.bIsActive && !instance->rActor->IsDying()
			&& instance->rActor->GetCurrentWeapon() == instance->rWeap)
		{
			if (auto biped = instance->rActor->GetValidBip01Names()) {
				if (auto weapNode = instance->rActor->pkBaseProcess->GetWeaponBone(biped);
					weapNode && weapNode->m_kChildren.m_usSize != 0)
				{
					st.bIsActive = true;
					st.uiTicksPassed = 0;
					InitializeHeatData(instance, instance->config);
					InitializeHeatFX(instance, instance->config);
					InitializeHeatSounds(instance, instance->config);
				}
			}
		}

		const float timePassed = st.uiTicksPassed * frameTime;

		//Store the previous state so that we can compare that to the current state.
		const bool wasOverheating = st.IsOverheating();

		if (instance->config->iOverchargeEffect & OCEffects_Overheat)
			st.UpdateOverheat();

		UpdateHeatFX(instance, frameTime);

		if (st.uiTicksPassed < 0xFFFF)
			++st.uiTicksPassed;

		//MenuMode check prevents you from charging up from within the pipboy, containers or dialogue. 
		//Overcharge and charge delay are only allowed for players because the NPCs are not equipped to perform tasks like these.  
		if (isPlayer && st.bIsActive && !MenuMode()) {
			if (!st.IsOverheating()) {
				if (st.iCanOverheat == 1)
					st.iCanOverheat = 2;
				UpdateOverchargeShot(instance, frameTime);
				UpdateChargeDelay(instance, frameTime);
			}
			else if (!wasOverheating)
				instance->fx.heatSoundHandle.FadeInPlay(100);
			else if (g_OCSettings.bOverheatLockout) OverheatLockout();
		}
		else if (isPlayer) {
			//Clear the effect states so that they don't get stored from within a menu.
			st.uiOCEffect &= ~OCEffects_Overcharge;
			st.uiOCEffect &= ~OCEffects_AltProjectile;
			st.uiOCEffect &= ~OCEffects_ChargeDelay;
			FadeOutAndStop(&instance->fx.chargeSoundHandle, 100);
			if (OCTranslate) {
				OCTranslate->m_kLocal.m_Translate.x = 0.0f;
				OCTranslate->m_kLocal.m_Translate.y = 0.0f;
			}
		}
		//We only clear memory whenever a weapon is inactive and a certain amount of time is passed. 
		if (!st.bIsActive && st.fHeatVal <= 0.0f && timePassed >= ERASE_DELAY) return false;
		if (!MenuMode() && timePassed >= COOLDOWN_DELAY && !(st.uiOCEffect & STOP_COOLDOWN_FLAGS) && st.fHeatVal > 0.0f)
			st.fHeatVal = (std::max)(0.0f, st.fHeatVal - frameTime * st.fCooldownRate);

		return true;
	}

	//Since NPCs don't require instant feedback, approximating frametime is close enough.
	void UpdateActiveOCWeapons()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;
		const float frameTimeAdjusted = frameTime * NPC_UPDATE_THROTTLE;

		activeOCWeapons.erase(
			std::remove_if(activeOCWeapons.begin(), activeOCWeapons.end(),
				[&](std::shared_ptr<HeatData>& inst)
				{
					if (inst->rActor == PlayerCharacter::GetSingleton())
						return false;

					else return !UpdateActiveWeapons(inst, frameTimeAdjusted);
				}),
			activeOCWeapons.end()
		);
	}

	//The player gets their own container because they require snappy instant feedback and is updated more frequently. 
	void UpdatePlayerOCWeapons()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;
		playerOCWeapons.erase(
			std::remove_if(playerOCWeapons.begin(), playerOCWeapons.end(),
				[&](std::shared_ptr<HeatData>& inst)
				{
					return !UpdateActiveWeapons(inst, frameTime, true);
				}),
			playerOCWeapons.end()
		);
	}

	void ParticleCleanup()
	{
		std::erase_if(activeInstances, [](const auto& inst) {
			auto heat = inst.second;
			return !heat || !heat->state.bIsActive;
			});
	}

	void ClearOCWeapons()
	{
		std::vector<std::shared_ptr<HeatData>>().swap(playerOCWeapons);
		playerOCWeapons.reserve(8);
		std::vector<std::shared_ptr<HeatData>>().swap(activeOCWeapons);
		activeOCWeapons.reserve(12);
		std::vector<NiParticleSystemPtr>().swap(worldSpaceParticles);
		worldSpaceParticles.reserve(24);
		std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>().swap(activeInstances);
		activeInstances.reserve(32);
		std::unordered_map<BSPSysSimpleColorModifier*, std::weak_ptr<HeatData>>().swap(colorModifiers);
		colorModifiers.reserve(32);
		std::unordered_map<BSPSysSimpleColorModifier*, ParticleInstance>().swap(colorMods);
		colorMods.reserve(32);
	}

	//Grabs OCWeapons from any currently active actors, initializes them, and stores for use.
	UInt32 __fastcall GetEquippedOCWeapon(Actor* rActor)
	{
		UInt32 weapon = ThisStdCall<UInt32>(0x8A1710, rActor);
		if (!rActor) return weapon;
		auto heat = GetOrCreateHeat(rActor);
		return weapon;
	}

	//Controls Fire Rate and Prevents firing on Overheat for both the player and actors
	//Null animations (0xFF) is the game's regular method of canceling animations, But attack queues must be clear.
	//Automatics, because they loop, they will constantly queue new attacks even if a null anim is played. 
	bool __fastcall PlayFireAnimation(Actor* rActor, void* edx, UInt8 groupID)
	{
		if (!rActor || !rActor->pkBaseProcess) return ThisStdCall<bool>(0x893A40, rActor, groupID);

		TESObjectWEAP* rWeap = rActor->GetCurrentWeapon();
		if (!rWeap) return ThisStdCall<bool>(0x893A40, rActor, groupID);
		const auto ogFireRate = rWeap->animAttackMult;

		bool result = false;

		if (auto heat = GetActiveHeat(rActor->uiFormID, rWeap->uiFormID))
		{
			if (g_OCSettings.bStats)
			{
				const float heatRatio = heat->state.fHeatVal / 100.0f;
				rWeap->animAttackMult = InterpolateBase(
					rWeap->animAttackMult,
					heat->config->fMinFireRate, heat->config->fMaxFireRate,
					heatRatio
				);
			}

			if (heat->state.uiOCEffect & STOP_FIRING_FLAGS)
			{
				rActor->pkBaseProcess->SetIsNextAttackLoopQueued(0);
				rActor->pkBaseProcess->SetForceFireWeapon(0);
				rActor->pkBaseProcess->SetIsFiringAutomaticWeapon(0);
				result = ThisStdCall<bool>(0x893A40, rActor, 0xFF);
				rWeap->animAttackMult = ogFireRate;
				return result;;
			}
		}
		result = ThisStdCall<bool>(0x893A40, rActor, groupID);
		rWeap->animAttackMult = ogFireRate;
		return result;
	}

	//Interpolating twice is needed since we don't permanently affect weapon stats.
	//Automatics must be handled both when the weapon is fired and when the animation plays.
	//Otherwise the anim loop will be sped up, projectile queue time is unchanged.
	double __fastcall GetWeapAnimMult(TESObjectWEAP* thisPtr, void* edx, char a2)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Actor* rActor = *reinterpret_cast<Actor**>(ebp + 0x8);

		if (!thisPtr || !rActor || !rActor->GetCurrentWeapon() || !g_OCSettings.bStats)
			return ThisStdCall<double>(0x646020, thisPtr, a2);

		const auto ogAnimAttackMult = thisPtr->animAttackMult;
		if (auto heat = GetActiveHeat(rActor->uiFormID, thisPtr->uiFormID))
		{
			const float heatRatio = heat->state.fHeatVal / 100.0f;
			thisPtr->animAttackMult = InterpolateBase(
				thisPtr->animAttackMult,
				heat->config->fMinFireRate, heat->config->fMaxFireRate,
				heatRatio
			);
		}
		double result = ThisStdCall<double>(0x646020, thisPtr, a2);
		thisPtr->animAttackMult = ogAnimAttackMult;
		return result;
	}

	__declspec(naked) UInt8 __cdecl GetAmmoRequiredForVats(int a1, char a2)
	{
		__asm
		{
			push	ebp
			mov		ebp, esp
			sub		esp, 0x38
			push    0x7F5056
			retn
		}
	}

	static inline UInt8 __cdecl CalcOCAmmoRequired(int a1, char a2)
	{
		const auto vats = VATS::GetSingleton();
		const auto player = PlayerCharacter::GetSingleton();
		const UInt8 ogBurst = GetAmmoRequiredForVats(a1, a2);
		if (!player || !vats || !g_OCSettings.bStats) return ogBurst;

		auto rWeap = player->GetCurrentWeapon();
		if (!rWeap) return ogBurst;

		UInt8 projectedAmmo = 0;
		if (auto heat = GetActiveHeat(player->uiFormID, rWeap->uiFormID)) {
			UInt32 shots = ogBurst / rWeap->ammoUse;
			if (shots == 0) return ogBurst;

			UInt32 totalShots = shots;
			for (auto tgt : vats->kTargetsList) {
				if (!tgt) continue;
				totalShots += shots;
			}

			float projectedHeat = heat->state.fHeatVal;
			for (UInt32 s = 1; s <= shots; ++s) {
				projectedHeat += heat->state.fHeatPerShot * ((totalShots - shots) + s);
				projectedAmmo += ScaleByPercentRange(
					rWeap->ammoUse,
					heat->config->iMinAmmoUsed,
					heat->config->iMaxAmmoUsed,
					projectedHeat / 100.0f
				);
			}
			if (projectedAmmo) return projectedAmmo;
		}
		return ogBurst;
	}

	//Controls the bulk of basic stat changes and increments heat.
	static inline void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, Actor* rActor)
	{
		auto player = PlayerCharacter::GetSingleton();
		auto vats = VATS::GetSingleton();
		auto heat = GetOrCreateHeat(rActor);
		if (!rWeap || !rActor || !heat)
		{
			ThisStdCall(0x523150, rWeap, rActor);
			return;
		}

		heat->state.HeatOnFire();

		if (rActor == player && heat->state.IsHot() && vats->IsPlaying())
			player->EndVatsKillCam(2, 0);

		//Backup Weapon Values - Backup needed since we're editing the baseform.
		const UInt8 ogAmmoUse = rWeap->ammoUse;
		const UInt8 ogProjectiles = rWeap->numProjectiles;
		const UInt16 ogDamage = rWeap->usAttackDamage;
		const UInt16 ogCritDamage = rWeap->criticalDamage;
		const float ogMinSpread = rWeap->minSpread;

		//Using ScaleByPercentRange() just in case a user has altered the base values at all.
		const HeatConfiguration* config = heat->config;
		const float hRatio = heat->state.fHeatVal / 100.0f;
		heat->state.uiAmmoUsed = ScaleByPercentRange(rWeap->ammoUse, config->iMinAmmoUsed, config->iMaxAmmoUsed, hRatio);
		heat->state.uiProjectiles = ScaleByPercentRange(rWeap->numProjectiles, config->iMinProjectiles, config->iMaxProjectiles, hRatio);
		heat->state.uiDamage = InterpolateBase(rWeap->usAttackDamage, config->fMinDamage, config->fMaxDamage, hRatio);
		heat->state.uiCritDamage = InterpolateBase(rWeap->criticalDamage, config->fMinCritDamage, config->fMaxCritDamage, hRatio);
		heat->state.fAccuracy = InterpolateBase(rWeap->minSpread, config->fMinSpread, config->fMaxSpread, hRatio);

		UpdatePerks(heat);

		if (g_OCSettings.bStats)
		{
			rWeap->ammoUse = heat->state.uiAmmoUsed;
			if (heat->state.uiOCEffect & OCEffects_AltProjectile)
				rWeap->ammoUse *= 3;

			rWeap->numProjectiles = heat->state.uiProjectiles;
			rWeap->usAttackDamage = heat->state.uiDamage;
			rWeap->criticalDamage = heat->state.uiCritDamage;
			rWeap->minSpread = heat->state.fAccuracy;
		}

		ThisStdCall(0x523150, rWeap, rActor);

		auto equippedAmmo = rActor->pkBaseProcess->GetAmmo();
		if (rActor == player && equippedAmmo
			&& equippedAmmo->iCountDelta == 0
			&& !rActor->GetPerkRank(OCPerkCriticalMass, 0))
			heat->state.iCanOverheat = 1;


		//Restore Weapon Values - Need to restore so only the current weapon is altered.
		rWeap->ammoUse = ogAmmoUse;
		rWeap->numProjectiles = ogProjectiles;
		rWeap->usAttackDamage = ogDamage;
		rWeap->criticalDamage = ogCritDamage;
		rWeap->minSpread = ogMinSpread;
	}

	//Controls the color of affected muzzle flashes
	static inline void __fastcall MuzzleFlashWrapper(MuzzleFlash* flash)
	{
		if (!flash || !flash->pSourceActor || !flash->pSourceWeapon || !g_OCSettings.bVFX)
		{
			ThisStdCall(0x9BB690, flash);
			return;
		}
		const NiNodePtr muzzleNode = flash->spNode;
		UInt32 sourceID = flash->pSourceActor->uiFormID;
		UInt32 weapID = flash->pSourceWeapon->uiFormID;
		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			if (flash->spLight) flash->spLight->SetDiffuseColor(heat->fx.currCol);
			if (muzzleNode)
			{
				TraverseNiNode<BSValueNode>(muzzleNode, [&heat](BSValueNodePtr valueNode) {
					activeInstances[valueNode] = heat;
					});
				TraverseNiNode<NiGeometry>(muzzleNode, [&heat](NiGeometryPtr geom) {
					CreateEmissiveColor(geom.m_pObject, heat->fx.currCol);
					});
				TraverseNiNode<NiParticleSystem>(muzzleNode, [&heat](NiParticleSystemPtr psys) {
					activeInstances[psys] = heat;
					});
			}
		}
		ThisStdCall(0x9BB690, flash);
	}

	//Controls projectile properties, handles alternate projectiles, and updates VFX
	static Projectile* __cdecl ProjectileWrapper(
		BGSProjectile* apBGSProjectile, Actor* apActor, CombatController* apCombatController, TESObjectWEAP* apWeap,
		NiPoint3 akPos, float afRotZ, float afRotX, NiNode* a10, TESObjectREFR* apLiveGrenadeTargetRef,
		bool abAlwaysHit, bool abIgnoreGravity, float afAngularMomentumZ, float afAngularMomentumX, TESObjectCELL* apCell)
	{
		UInt32 actorID = apActor ? apActor->uiFormID : 0;
		UInt32 weapID = apWeap ? apWeap->uiFormID : 0;
		auto heat = GetActiveHeat(actorID, weapID);
		auto ogProj = apBGSProjectile;

		if (heat && apActor == PlayerCharacter::GetSingleton()
			&& heat->state.uiOCEffect & OCEffects_AltProjectile
			&& heat->config->iAltProjectileID != 0xFFFFFF) {
			heat->state.uiOCEffect &= ~OCEffects_AltProjectile;
			TESForm* altForm = TESForm::GetByID(heat->config->iAltProjectileID);
			BGSProjectile* altProj = reinterpret_cast<BGSProjectile*>(altForm);
			apBGSProjectile = altProj;
		}
		else if (heat && heat->state.uiOCEffect & OCEffects_AltProjectile)
			heat->state.uiOCEffect &= ~OCEffects_AltProjectile;


		Projectile* proj = CdeclCall<Projectile*>(0x9BCA60,
			apBGSProjectile, apActor, apCombatController, apWeap,
			akPos, afRotZ, afRotX, a10, apLiveGrenadeTargetRef,
			abAlwaysHit, abIgnoreGravity, afAngularMomentumZ, afAngularMomentumX, apCell);

		apBGSProjectile = ogProj;
		const NiNodePtr projNode = proj ? proj->Get3D() : nullptr;
		if (!heat || !projNode) return proj;

		//Update Projectile Values - Don't need to backup because they're references.

		if (g_OCSettings.bStats)
		{
			const float heatRatio = heat->state.fHeatVal / 100.0f;
			projNode->m_kLocal.m_fScale =
				InterpolateBase(
					projNode->m_kLocal.m_fScale,
					heat->config->fMinProjectileSize,
					heat->config->fMaxProjectileSize,
					heatRatio
				);
			heat->state.fProjectileSize = projNode->m_kLocal.m_fScale;

			proj->fSpeedMult =
				InterpolateBase(
					proj->fSpeedMult,
					heat->config->fMinProjectileSpeed,
					heat->config->fMaxProjectileSpeed,
					heatRatio
				);
			heat->state.fProjectileSpeed = proj->fSpeedMult;
		}

		if (g_OCSettings.bVFX)
		{
			//Update Current Color - Need to update early since it races UpdateHeatFX() for current color.
			heat->fx.currCol = SmoothColorShift(heat->state.fHeatVal, heat->config->iMinColor, heat->config->iMaxColor);

			//Insert Value Nodes to activeInstances - BSValueNodes serve as emitter objects for their respective particles.
			TraverseNiNode<BSValueNode>(projNode, [&heat](BSValueNodePtr valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(projNode, [&heat](NiGeometryPtr geom) {
				CreateEmissiveColor(geom.m_pObject, heat->fx.currCol);
				});
			TraverseNiNode<NiParticleSystem>(projNode, [&heat](NiParticleSystemPtr psys) {
				activeInstances[psys] = heat;
				});
		}
		return proj;
	}

	//Modifies spawned temporary impact effect before they are spawned.
	static BSTempEffectParticle* __cdecl ImpactWrapper(
		TESObjectCELL* cell, float lifetime, const char* fileName,
		NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B8);
		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);

		if (!impact || !impact->spParticleObject || !pProjectile || !pProjectile->pSourceRef || !pProjectile->pSourceWeapon) return impact;

		const NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		if (!impactNode || !g_OCSettings.bVFX) return impact;

		UInt32 sourceID = pProjectile->pSourceRef->uiFormID;
		UInt32 weapID = pProjectile->pSourceWeapon->uiFormID;
		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			//Insert Value Nodes to activeInstances - BSValueNodes serve as emitter objects for their respective particles.
			TraverseNiNode<BSValueNode>(impactNode, [&heat](BSValueNodePtr valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(impactNode, [&heat](NiGeometryPtr geom) {
				CreateEmissiveColor(geom.m_pObject, heat->fx.currCol);
				});
			TraverseNiNode<NiParticleSystem>(impactNode, [&heat](NiParticleSystemPtr psys) {
				activeInstances[psys] = heat;
				});
		}
		return impact;
	}

	//Controls spawned temporary impact effect before they are spawned and applies applicable object effects
	static BSTempEffectParticle* __cdecl ImpactActorWrapper(
		TESObjectCELL* cell, float lifetime, const char* fileName,
		NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		uintptr_t ebxVal;
		__asm mov ebxVal, ebx

		ActorHitData* hitData = *reinterpret_cast<ActorHitData**>(ebxVal + 0xC);
		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);

		if (!impact || !impact->spParticleObject || !hitData->pkSource || !hitData->pkTarget || !hitData->pWeapon) return impact;

		const NiNodePtr impactNode = impact->spParticleObject->IsNiNode();
		if (!impactNode) return impact;

		UInt32 sourceID = hitData->pkSource->uiFormID;
		UInt32 weapID = hitData->pWeapon->uiFormID;
		if (auto heat = GetActiveHeat(sourceID, weapID))
		{
			if (hitData->pkTarget && heat->config->iObjectEffectID
				&& heat->state.fHeatVal >= heat->config->iObjectEffectThreshold) {
				TESForm* effect = TESForm::GetByID(heat->config->iObjectEffectID);
				if (effect) hitData->pkTarget->CastSpellImmediate(reinterpret_cast<MagicItemForm*>(effect), 0, hitData->pkTarget, 1, 0);
			}

			if (!g_OCSettings.bVFX) return impact;

			//Insert Value Nodes to activeInstances - BSValueNodes serve as emitter objects for their respective particles.
			TraverseNiNode<BSValueNode>(impactNode, [&heat](BSValueNodePtr valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(impactNode, [&heat](NiGeometryPtr geom) {
				CreateEmissiveColor(geom.m_pObject, heat->fx.currCol);
				});
			TraverseNiNode<NiParticleSystem>(impactNode, [&heat](NiParticleSystemPtr psys) {
				activeInstances[psys] = heat;
				});
		}
		return impact;
	}

	//Controls Explosion VFX as they are spawning in.
	static void __fastcall ExplosionWrapper(Explosion* thisPtr)
	{
		ThisStdCall(0x9B1260, thisPtr);
		if (!thisPtr || !thisPtr->pOwnerRef || !thisPtr->pOwnerRef->IsActor()) return;

		Actor* owner = reinterpret_cast<Actor*>(thisPtr->pOwnerRef);
		UInt32 weapID = owner->GetCurrentWeaponID();

		NiNodePtr explNode = thisPtr->Get3D();
		if (!explNode || !g_OCSettings.bVFX) return;

		if (auto heat = GetActiveHeat(owner->uiFormID, weapID))
		{
			heat->fx.currCol = SmoothColorShift(heat->state.fHeatVal, heat->config->iMinColor, heat->config->iMaxColor);
			if (thisPtr->spLight) thisPtr->spLight->SetDiffuseColor(heat->fx.currCol);
			TraverseNiNode<BSValueNode>(explNode, [&heat](BSValueNodePtr valueNode) {
				activeInstances[valueNode] = heat;
				});
			TraverseNiNode<NiGeometry>(explNode, [&heat](NiGeometryPtr geom) {
				CreateEmissiveColor(geom.m_pObject, heat->fx.currCol);
				});
			TraverseNiNode<NiParticleSystem>(explNode, [&heat](NiParticleSystemPtr psys) {
				activeInstances[psys] = heat;
				});
		}
	}

	//Controls Kill Effect Shader VFX
	static MagicShaderHitEffect* __fastcall MagicShaderVFXWrapper(
		MagicShaderHitEffect* thisPtr, void* edx, TESObjectREFR* target,
		TESEffectShader* a3, float duration)
	{
		MagicShaderHitEffect* mshe = ThisStdCall<MagicShaderHitEffect*>(0x81F580, thisPtr, target, a3, duration);

		if (!target->IsActor() || !g_OCSettings.bVFX) return mshe;
		Actor* targetActor = reinterpret_cast<Actor*>(target);

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

	//Controls the color of ash/goo piles. 
	static TESObjectREFR* __fastcall GooPileWrapper(
		TESDataHandler* thisPtr, void* edx, TESBoundObject* pObject,
		NiPoint3* apLocation, NiPoint3* apDirection, TESObjectCELL* pInterior,
		TESWorldSpace* pWorld, TESObjectREFR* pReference,
		void* pAddPrimitive, void* pAdditionalData)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Actor* targetActor = *reinterpret_cast<Actor**>(ebp + 0x20);

		TESObjectREFR* expl = ThisStdCall<TESObjectREFR*>(
			0x4698A0, thisPtr, pObject, apLocation,
			apDirection, pInterior, pWorld, pReference,
			pAddPrimitive, pAdditionalData);

		if (!targetActor || !targetActor->pKiller || !g_OCSettings.bVFX) return expl;

		Actor* killer = targetActor->pKiller;
		UInt32 killerID = killer->uiFormID;
		UInt32 killerWeapID = killer->GetCurrentWeaponID();

		//Ash/Goo piles have their vertex colors desaturated earlier in the process and now use emissive colors. 
		if (auto heat = GetActiveHeat(killerID, killerWeapID))
		{
			//LoadGraphics() is called earlier than the game usually would so we can control the pile's color.
			if (NiNodePtr node = pObject->LoadGraphics(expl))
			{
				TraverseNiNode<BSValueNode>(node, [&heat](BSValueNodePtr valueNode) {
					activeInstances[valueNode] = heat;
					});
				TraverseNiNode<NiGeometry>(node, [&heat](NiGeometryPtr geom) {
					CreateEmissiveColor(geom.m_pObject, heat->fx.currCol);
					});
				TraverseNiNode<NiParticleSystem>(node, [&heat](NiParticleSystemPtr psys) {
					activeInstances[psys] = heat;
					});
			}
		}
		return expl;
	}


	__declspec(naked) void __fastcall EmitterEmitParticles(NiPSysEmitter* thisPtr, void* edx, float fTime, UInt16 usNumParticles, const float* pfAges)
	{
		__asm
		{
			sub     esp, 0x50
			push    ebx
			push    esi
			push    0xC220C5
			retn
		}
	}

	//Controls active color modifiers, so they don't break either vertex or emissive colors. 
	static void __fastcall ReplaceColorModifiers(NiPSysEmitter* thisPtr, void* edx, float fTime, UInt16 usNumParticles, const float* pfAges)
	{
		if (!thisPtr || !thisPtr->m_pkTarget || !g_OCSettings.bVFX)
		{
			EmitterEmitParticles(thisPtr, edx, fTime, usNumParticles, pfAges);
			return;
		}

		//Finding the emitter type this way as to cover both volume emitters as well as other general emitters. This is primarily for explosions. 
		NiPSysVolumeEmitterPtr volEmit = thisPtr->IsNiType<NiPSysVolumeEmitter>()
			? static_cast<NiPSysVolumeEmitter*>(thisPtr)
			: nullptr;

		NiAVObjectPtr key = volEmit ? volEmit->m_pkEmitterObj : thisPtr->m_pkTarget;

		if (auto it = activeInstances.find(key); it != activeInstances.end())
		{
			//Bright or more opaque particles should typically use a DstBlendMode of 'One' so checking it filters particles like smoke or dust.
			NiAlphaPropertyPtr alpha = thisPtr->m_pkTarget->GetAlphaProperty();
			if (!alpha || !alpha->IsDstBlendMode(NiAlphaProperty::ALPHA_ONE))
				activeInstances.erase(it);
			else if (!it->second)
			{
				thisPtr->m_kInitialColor = it->second->fx.currCol;
				for (const auto& modifier : thisPtr->m_pkTarget->m_kModifierList)
				{
					//Adding the active color modifier to target it for a later detour of its update.
					if (BSPSysSimpleColorModifierPtr scm = modifier->NiDynamicCast<BSPSysSimpleColorModifier>())
						colorModifiers[scm] = it->second;
				}
				//Disabling emissive colors so the vertex colors control the appearance.
				if (thisPtr->m_pkTarget->GetMaterialProperty())
					SetEmissiveColor(thisPtr->m_pkTarget, NiColor(1, 1, 1));
			}
		}
		EmitterEmitParticles(thisPtr, edx, fTime, usNumParticles, pfAges);
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

	//Detour Original Function - We ONLY update alphas otherwise the color modifiers will reset vertex colors every time Update() is called.
	static inline void __fastcall ColorModifierUpdateWrapper(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData)
	{
		if (!apData->m_pkColor || !g_OCSettings.bVFX) {
			OriginalColorModifierUpdate(apThis, edx, afTime, apData);
			return;
		}

		if (auto it = activeParticles.find(apData); it != activeParticles.end())
		{
			float timeColor2 = apThis->fColor2Start - apThis->fColor1End;
			float timeColor3 = apThis->fColor3Start - apThis->fColor2End;
			for (int i = 0; i < apData->m_usActiveVertices; ++i)
			{
				NiColorA* pColor = apData->m_pkColor;
				NiParticleInfo* pInfo = &apData->m_pkParticleInfo[i];
				float lifePercent = pInfo->m_fAge / pInfo->m_fLifeSpan;

				NiColor color1(apThis->kColor1.r, apThis->kColor1.g, apThis->kColor1.b);
				NiColor color2(apThis->kColor2.r, apThis->kColor2.g, apThis->kColor2.b);
				NiColor color3(apThis->kColor3.r, apThis->kColor3.g, apThis->kColor3.b);

				if (it->second.contains(i)) {

					if (apThis->fFadeOut != 0.0 && apThis->fFadeOut < lifePercent) {
						float fadeOutPercent = (lifePercent - apThis->fFadeOut) / (1.0 - apThis->fFadeOut);
						pColor[i].a = (apThis->kColor3.a - apThis->kColor2.a) * fadeOutPercent + apThis->kColor2.a;
					}
					else if (apThis->fFadeIn == 0.0 || apThis->fFadeIn <= lifePercent) {
						pColor[i].a = apThis->kColor2.a;
					}
					else {
						float fadeInPercent = lifePercent / apThis->fFadeIn;
						pColor[i].a = (apThis->kColor2.a - apThis->kColor1.a) * fadeInPercent + apThis->kColor1.a;
					}
					continue; 
				}

				if (timeColor2 != 0 && apThis->fColor2Start > lifePercent) {
					if (apThis->fColor1End <= lifePercent) {
						float timeLeft1 = (lifePercent - apThis->fColor1End) / timeColor2;
						pColor[i].r = (color2.r - color1.r) * timeLeft1 + color1.r;
						pColor[i].g = (color2.g - color1.g) * timeLeft1 + color1.g;
						pColor[i].b = (color2.b - color1.b) * timeLeft1 + color1.b;
					}
					else {
						pColor[i].r = color1.r;
						pColor[i].g = color1.g;
						pColor[i].b = color1.b;
					}
				}
				else if (timeColor3 == 0.0 || apThis->fColor2End >= lifePercent) {
					pColor[i].r = color2.r;
					pColor[i].g = color2.g;
					pColor[i].b = color2.b;
				}
				else if (apThis->fColor3Start < lifePercent) {
					pColor[i].r = color3.r;
					pColor[i].g = color3.g;
					pColor[i].b = color3.b;
				}
				else {
					float timeLeft2 = (lifePercent - apThis->fColor2End) / timeColor3;
					pColor[i].r = (color3.r - color2.r) * timeLeft2 + color2.r;
					pColor[i].g = (color3.g - color2.g) * timeLeft2 + color2.g;
					pColor[i].b = (color3.b - color2.b) * timeLeft2 + color2.b;
				}
				if (apThis->fFadeOut != 0.0 && apThis->fFadeOut < lifePercent) {
					float fadeOutPercent = (lifePercent - apThis->fFadeOut) / (1.0 - apThis->fFadeOut);
					pColor[i].a = (apThis->kColor3.a - apThis->kColor2.a) * fadeOutPercent + apThis->kColor2.a;
				}
				else if (apThis->fFadeIn == 0.0 || apThis->fFadeIn <= lifePercent) {
					pColor[i].a = apThis->kColor2.a;
				}
				else {
					float fadeInPercent = lifePercent / apThis->fFadeIn;
					pColor[i].a = (apThis->kColor2.a - apThis->kColor1.a) * fadeInPercent + apThis->kColor1.a;
				}
			}
		}
		else
		{
			OriginalColorModifierUpdate(apThis, edx, afTime, apData);
			return;
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

	//Detour the original function since the game typically zeros out both translation and rotation for particles when viewed in first person. 
	//But it isn't viewable in first person. So we anchor the position to the first person node so particles can have screen space simulation.
	//Because the particle relative to the camera, there is only simulation when the position relative to the camera is changed.
	static inline void __fastcall FirstPersonPsysWorldUpdate(NiParticleSystem* thisPtr, void* edx, NiUpdateData* apData)
	{
		if (!thisPtr || !apData || !thisPtr->m_bWorldSpace || !ContainsValue(worldSpaceParticles, thisPtr) || !g_OCSettings.bVFX)
		{
			UpdatePsysWorldData(thisPtr, edx, apData);
			return;
		}
		else if (auto camera1st = PlayerCharacter::GetSingleton()->GetPlayerNode(1))
		{
			ThisStdCall(0xA68C60, thisPtr, apData);
			memcpy(&thisPtr->m_kUnmodifiedWorld, &thisPtr->m_kWorld, sizeof(thisPtr->m_kUnmodifiedWorld));
			thisPtr->m_kWorld.m_Translate = camera1st->m_kWorld.m_Translate;
			memcpy(&thisPtr->m_kWorld.m_Rotate, &NiMatrix3::IDENTITY, sizeof(NiMatrix3));
		}
	}

	//Clear out any tracked color modifiers and color controlled objects. 
	static void __stdcall ClearTrackedEmitters(NiAVObject* toDelete)
	{
		activeInstances.erase(toDelete);
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

	static void __fastcall InitializeParticleWrapper(NiParticleSystem* thisPtr, void*, UInt16 usNewParticle)
	{
		uintptr_t esiVal = 0;
		__asm mov esiVal, ESI;

		auto* emitter = reinterpret_cast<NiPSysEmitter*>(esiVal);
		if (!emitter) {
			ThisStdCall(0xC1AEE0, thisPtr, usNewParticle);
			return;
		}
		auto volEmit = emitter->NiDynamicCast<NiPSysVolumeEmitter>();
		if (!volEmit || !volEmit->m_pkEmitterObj) {
			ThisStdCall(0xC1AEE0, thisPtr, usNewParticle);
			return;
		}
		auto it = activeInstances.find(volEmit->m_pkEmitterObj);
		if (it == activeInstances.end()) {
			ThisStdCall(0xC1AEE0, thisPtr, usNewParticle);
			return;
		}
		auto alpha = thisPtr->GetAlphaProperty();
		if (!alpha || !alpha->IsDstBlendMode(NiAlphaProperty::ALPHA_ONE)) {
			activeInstances.erase(it);
			ThisStdCall(0xC1AEE0, thisPtr, usNewParticle);
			return;
		}

		auto instPtr = it->second;
		if (!instPtr) {
			activeInstances.erase(it);
		}
		else {
			NiPSysDataPtr psysData = (NiPSysData*)thisPtr->GetModelData();
			auto mapIt = activeParticles.find(psysData);
			if (mapIt == activeParticles.end()) {
				mapIt = activeParticles.emplace(psysData, std::unordered_set<UInt16>{}).first;
			}
			mapIt->second.insert(usNewParticle);
			psysData->m_pkColor[usNewParticle] = instPtr->fx.currCol;
			SetEmissiveColor(thisPtr, NiColor(1, 1, 1));
		}
		ThisStdCall(0xC1AEE0, thisPtr, usNewParticle);
	}

	static inline void __fastcall RemoveParticleWrapper(NiPSysData* thisPtr, void* edx, UInt16 usParticle)
	{
		if (auto it = activeParticles.find(thisPtr); it != activeParticles.end())
		{
			UInt16 removalSlot = thisPtr->m_usActiveVertices - 1;

			if (usParticle == removalSlot) {
				it->second.erase(usParticle);
			}
			else {
				it->second.erase(removalSlot);
			}
		}
		ThisStdCall(0xA964C0, thisPtr, usParticle);
	}

	void InitHooks()
	{
		WriteRelCall(0x888C23, &GetEquippedOCWeapon);				//0x888C23 - Actor::GetEquippedWeapon() (via Actor::Update())
		WriteRelCall(0x949CF1, &PlayFireAnimation);					//0x949CF1 - Actor::FiresWeapon_893A40() (via PlayerCharacter::Attack_948310())
		WriteRelCall(0x8BA7BF, &PlayFireAnimation);					//0x8BA7BF - Actor::FiresWeapon_893A40() (via Actor::HandleQueuedIdleFlags())
		WriteRelCall(0x929F5D, &GetWeapAnimMult);					//0x929F5D - TESObjectWEAP::GetAnimAttackMult() (via MiddleHighProcess::Func010E())
		WriteRelCall(0x8BADE9, &FireWeaponWrapper);					//0x8BADE9 - TESObjectWEAP::HandleFiredWeapon() (via Actor::QueuedIdleFlags())
		WriteRelCall(0x9BB7CD, &MuzzleFlashWrapper);				//0x9BB7CD - MuzzleFlash::Enable() (via Projectile::EnableMuzzleFlash())
		WriteRelCall(0x5245BD, &ProjectileWrapper);					//0x5245BD - BGSProjectile::CreateProjectile() (via TESObjectWEAP::HandleFiredWeapon())
		WriteRelCall(0x9C2AC3, &ImpactWrapper);						//0x9C2AC3 - BSTempEffectParticle::Create() (via Projectile::SpawnCollisionEffects())
		WriteRelCall(0x88F245, &ImpactActorWrapper);				//0x88F245 - BSTempEffectParticle::Create() (via  Actor::CreateBlood())
		WriteRelCall(0x9AD024, &ExplosionWrapper);					//0x9AD024 - Explosion::CreateLight() (via Explosion::CheckInit3D())
		WriteRelCall(0x5D1C90, &MagicShaderVFXWrapper);				//0x5D1C90 - MagicShaderHitEffect::MagicShaderHitEffect_() (via Cmd_PlayMagicShaderVisuals_Execute())
		WriteRelCall(0x5DBD56, &GooPileWrapper);					//0x5DBD56 - TESDataHandler::CreateReferenceAtLocation() (via Script::AttachAshPileFunction())

		WriteRelJump(0x7F5050, &CalcOCAmmoRequired);

		//WriteRelJump(0xC220C0, &ReplaceColorModifiers);				//0xC220C0 - From NiPSysEmitter::EmitParticles()
		WriteRelJump(0xC602E0, &ColorModifierUpdateWrapper);		//0xC602E0 - From BSPSysSimpleColorModifier::Update()
		WriteRelJump(0xC1ADD0, &FirstPersonPsysWorldUpdate);		//0xC1ADD0 - From NiParticleSystem::UpdateWorldData()
		WriteRelJump(0xC5E164, &ClearUnrefEmittersHook);			//0xC5E164 - From BSMasterParticleSystem::ClearUnreferencedEmitters()

		WriteRelCall(0xC2237A, &InitializeParticleWrapper);
		WriteRelCall(0xC24CD3, &RemoveParticleWrapper);
	}
}