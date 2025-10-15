#pragma once

#include "ActorMover.hpp"
#include "ActorValueOwner.hpp"
#include "CachedValuesOwner.hpp"
#include "ExtraContainerChanges.hpp"
#include "MagicCaster.hpp"
#include "MagicTarget.hpp"
#include "MobileObject.hpp"
#include "ModifierList.hpp"

struct FurnitureMark;
class TESTopic;
class BGSPerk;
class bhkRagdollController;
class bhkRagdollPenetrationUtil;
class TESActorBase;
class ContinuousBeamProjectile;
class TESPackage;
class TESRace;
class CombatGroup;
class CombatController;
class BGSEntryPointPerkEntry;
class BGSPerkEntry;
class SpellItem;
class ArrowProjectile;
class BSSoundHandle;
class ItemChange;
class TESObjectBOOK;

typedef std::vector<TESForm*> EquippedItemsList;

class Actor : public MobileObject, public MagicCaster, public MagicTarget, public ActorValueOwner, public CachedValuesOwner {
public:
	Actor();
	virtual ~Actor();

	virtual bool							IsGuard() const;
	virtual void							SetGuard(bool abVal);
	virtual void							SetFlyInventoryWeight(float afVal);
	virtual float							GetFlyInventoryWeight() const;
	virtual int32_t							GetInfamy() const;
	virtual int32_t							GetFame() const;
	virtual void							SetIgnoreCrime(bool abVal);
	virtual bool							GetIgnoreCrime() const;
	virtual void							Resurrect(bool abResetInventory, bool abAttach3D, bool abGetupResurect);
	virtual void							SetAdvanceNumber(uint32_t auiVal);
	virtual uint32_t						GetAdvanceNumber() const;
	virtual void							SetSkillAdvanceDelayed(ActorValue::Index aeIndex);
	virtual ActorValue::Index				GetSkillAdvanceDelayed() const;
	virtual bool							DoDamage(float afHealth, float afFatigue, Actor* apSource);
	virtual void							DamageActionPoints(float afAmount);
	virtual void							DamageActionPointsForAction(uint32_t auiAction);
	virtual uint32_t						GetDisposition(Actor* apTarget, void* apOverride = nullptr);
	virtual void							UpdateMovement(float afTimeDelta, bool);
	virtual void							UpdateNonRenderSafe(float afTimeDelta, bool);
	virtual void							UpdateAnimMovement();
	virtual float							GetTurningSpeed() const;
	virtual bool							IsOverEncumbered() const;
	virtual bool							Unk_D7() const;
	virtual bool							IsPc() const;
	virtual bool							GetCannibal() const;
	virtual void							SetCannibal(bool abVal);
	virtual bool							GetSandMan() const;
	virtual void							SetSandMan(bool abVal);
	virtual void							InitiateSandManPackage(Actor* apTarget, TESObjectREFR* apFurnitureRef, FurnitureMark* apFurnitureMark, uint8_t aucFurnitureMarkerIndex);
	virtual void							InitiateCannibalPackage(Actor* apTarget);
	virtual TESRace*						GetRace() const;
	virtual float							GetReach() const;
	virtual void							SetRefraction(bool abEnable, float afRefractionPower);
	virtual void							SetHasRagdoll(bool abVal);
	virtual bool							GetHasRagdoll() const;
	virtual uint32_t						GetActorType() const;
	virtual void							SetActorValueF(uint32_t auiAVCode, float afValue);
	virtual void							SetActorValueI(uint32_t auiAVCode, int32_t aiValue);
	virtual void							MagicModActorValueF(uint32_t auiAVCode, float afModifier, Actor* apAttacker);
	virtual void							MagicModActorValueI(uint32_t auiAVCode, int32_t aiModifier, Actor* apAttacker);
	virtual void							ScriptModActorValueF(uint32_t auiAVCode, float afModifier, Actor* apAttacker);
	virtual void							ScriptModActorValueI(uint32_t auiAVCode, int32_t aiModifier, Actor* apAttacker);
	virtual void							GameModActorValueF(uint32_t auiAVCode, float afModifier, Actor* apAttacker);
	virtual void							GameModActorValueI(uint32_t auiAVCode, int32_t aiModifier, Actor* apAttacker);
	virtual void							ModActorBaseValueF(uint32_t auiAVCode, float afModifier);
	virtual void							ModActorBaseValueI(uint32_t auiAVCode, int32_t aiModifier);
	virtual ItemChange*						GetBestWeapon(uint32_t aeWeaponType) const;
	virtual ItemChange*						GetBestAmmo() const;
	virtual void							ResetArmorRating();
	virtual bool							DamageEquipment(ItemChange* apEquipment, float afDamage, bool abIgnoreArmorSkill = false);
	virtual TESObjectREFR*					DropObject(TESForm* apItem, ExtraDataList* apExtraList, int32_t aiCount, const NiPoint3* apPoint, const NiPoint3* apRotate);
	virtual void							PickUpObject(TESObjectREFR* apObject, int32_t aiCount, bool abPlayPickUpSounds);
	virtual void							CastScroll(TESObjectBOOK* apBook, MagicTarget* apTarget);
	virtual void							CheckCastWhenStrikesEnchantment(ItemChange* apWeapon, Actor* apTarget, ArrowProjectile* apArrow, bool& abInsufficientCharge);
	virtual void							UpdateWornEnchantments(float afElapsedTime);
	virtual bool							AddSpell(SpellItem* apSpell);
	virtual bool							RemoveSpell(SpellItem* apSpell);
	virtual bool							Reload(TESObjectWEAP* apWeapon, uint32_t, bool);
	virtual bool							ReloadAlt(TESObjectWEAP* apWeapon, uint32_t, bool, bool);
	virtual void							UseAmmo(uint32_t auiCount = UINT32_MAX);
	virtual void							CleanOrphanedMagicModifiers();
	virtual CombatGroup*					GetCombatGroup() const;
	virtual void							SetCombatGroup(CombatGroup* apGroup);
	virtual bool							InitiateTresPassPackage(void* apPackage);
	virtual void							RunUpdates(float afDelta);
	virtual BSSoundHandle					VoiceSoundFunctionEx(TESTopic* apTopic, Actor* apTarget, bool abIs2D, bool abNeverSubtitle, int unused, bool abSayTo);
	virtual void							InitiateSpectator();
	virtual void							InitiateFlee(TESObjectREFR* apFleeRef, bool abRunOnce, bool abKnowsTarget, bool abCombatMode, TESObjectCELL* apCell, TESObjectREFR* apRef, float afFleeFromDist, float afFleeToDist);
	virtual void							InitiateSearch(TESObjectREFR* apAttacker, NiPoint3);
	virtual void							InitiateGetUpPackage();
	virtual void							SetAlpha(float afValue);
	virtual float							GetAlpha();
	virtual void							StartCombat(Actor* apTarget, CombatGroup* apCombatGroup, bool abForceCombat, bool abIsAggressor, bool abFleeing, int32_t aiPriority, bool abCastSpell, TESPackage* apPackage);
	virtual CombatController*				GetCombatController() const;
	virtual Actor*							GetCombatTarget() const;
	virtual void							UpdateCombat(float afElapsedTime);
	virtual void							StopCombat(Actor* apAttacker);
	virtual void							PerformOneRoundofCombat();
	virtual float							GetArmorDamageResistance(); // Officially probably still GetArmorRating
	virtual float							GetArmorDamageThreshold();
	virtual float							GetAttackDamage() const;
	virtual bool							IsTrespassing() const;
	virtual void							SetTrespassing();
	virtual bool							Unk_114() const;
	virtual void							Unk_115();
	virtual float							CalculateWalkSpeed();
	virtual float							CalculateRunSpeed();
	virtual void							ModifyDispositionTowardActor(Actor* apActor, float afValue);
	virtual float							GetDispositionModifierTowardActor(Actor* apActor) const;
	virtual void							RemoveDispositionModifierForActor(Actor* apActor);
	virtual void							SetActorStartingPosition();
	virtual bool							HasBeenAttacked() const;
	virtual void							SetBeenAttacked(bool abVal);
	virtual void							HitMe(ActorHitData* apHitData, float afHealthDmg, float afBlockDTMod, NiPoint3& unk, NiPoint3& arWeaponPos);
	virtual void							UseSkill0(ActorValue::Index aeSkill, uint32_t);
	virtual void							UseSkill1(ActorValue::Index aeSkill, uint32_t aeAction, float afModifier);
	virtual void							UseSkill2(ActorValue::Index aeSkill, float afUsage, void* apSkill);
	virtual void							RewardXP();
	virtual float							InternalGetActorValue(ActorValue::Index aeIndex, bool& arFound);
	virtual bool							InternalSetActorValue(ActorValue::Index aeIndex, float afValue);
	virtual void							UpdateWeaponWobble();
	virtual void							SetPerkRank(BGSPerk* apPerk, uint8_t aucRank, bool abTeammate);
	virtual void							RemovePerk(BGSPerk* apPerk, bool abTeammate);
	virtual uint8_t							GetPerkRank(BGSPerk* apPerk, bool abTeammate);
	virtual void							AddPerkEntryPoint(BGSEntryPointPerkEntry* apPerkEntry, bool abTeammate);
	virtual void							RemovePerkEntry(BGSEntryPointPerkEntry* apPerkEntry, bool abTeammate);
	virtual BSSimpleList<BGSPerkEntry*>*	GetPerkEntryListByID(uint8_t ucID, bool abTeammate) const;
	virtual void							StartAnimOn1stPerson(uint16_t aeAnimGroup, uint32_t aeActionFlags);
	virtual bool							IsImmobileCreature() const;
	virtual void							HandleHealthDamage(Actor* apAttacker, float afDamage);
	virtual void							AttackedBy(Actor* apAttacker, ActiveEffect* apEffect = nullptr);
	virtual void							HandleBlockedAttack(float afFullDamage, float afPercentageBlocked, Actor* apBlockingActor, ArrowProjectile* apArrow);
	virtual void							PrecacheData();
	virtual void							ProcessTracking(float afTime);
	virtual void							UpdateHeadTrackingEmotions(int empty = 0);
	virtual void							CreateActorMover();
	virtual void							DestroyActorMover();
	virtual NiPoint3						GetAnticipatedLocation(float afTime) const;

	struct Disposition {
		float	fValue;
		Actor* pTarget;
	};

	bhkRagdollController*		pRagdollController;
	bhkRagdollPenetrationUtil*	pRagdollPenetrationUtil;
	UInt32						unk0B4;
	float						flt0B8;
	bool						bIsAIProcessing;
	UInt8						byte0BD;
	UInt8						byte0BE;
	UInt8						byte0BF;
	Actor*						pKiller;
	UInt8						byte0C4;
	UInt8						byte0C5;
	UInt8						byte0C6;
	UInt8						byte0C7;
	float						fTime0C8;
	float						fCheckDeadBodyTimer;
	ModifierList				kForceAVList;
	ModifierList				kSetAVList;
	UInt8						byte0F0;
	UInt8						byte0F1;
	UInt8						byte0F2;
	UInt8						byte0F3;
	BSSimpleList<UInt32>		list0F4;
	BSSimpleList<Disposition*>	kDispositions;
	bool						bIsInCombat;
	Bitfield8					ucJIPActorFlags1;
	Bitfield8					ucJIPActorFlags2;
	Bitfield8					ucJIPActorFlags3;
	UInt32						uiLifeState;
	UInt32						uiCriticalStage;
	UInt32						uiQueuedAttackAnim;
	float						fPainSoundTimer;
	UInt8						byte118;
	UInt8						byte119;
	UInt16						usJIP11A;
	Bitfield32					uiFlags11C;
	UInt32						hoursPassed120;
	bool						bForceRun;
	bool						bForceSneak;
	bool						byte126;
	bool						bHasSearch;
	Actor*						pCombatTarget;
	BSSimpleArray<Actor*>*		pCombatTargets;
	BSSimpleArray<Actor*>*		pCombatAllies;
	UInt8						byte134;
	UInt8						byte135;
	UInt16						usJIP136;
	UInt32						unk138;
	UInt32						uiMinorCrimeCount;
	Bitfield32					uiActorFlags;
	bool						bIgnoreCrime;
	bool						byte145;
	bool						bIsResetAI;
	bool						byte147;
	TESActorBase*				pActorbase148;
	bool						bInWater;
	bool						bIsSwimming;
	bool						bAddToShadowSceneNode;
	bool						gap14F;
	UInt32						iActionValue;
	float						fTimerOnAction;
	float						fUpdateTargetTimer;
	UInt8						byte15C;
	UInt8						bShouldRotateToTrack;
	UInt16						usJIP15E;
	NiPoint3					kStartingPos;
	float						fStartingZRot;
	TESForm*					pStartingWorldOrCell;
	UInt8						byte174;
	UInt8						byte175;
	UInt16						usJIP176;
	float						fCachedSpread_Projectile;
	float						fCachedSpread_Scoped;
	float						fCachedSpread_WeaponModel;
	float						fCachedSpread_VATS;
	float						flt188;
	UInt8						isCameraRagdollAnimDist;
	bool						bIsTeammate;
	bool						byte18E;
	bool						byte18F;
	ActorMover*					pActorMover;
	UInt32						unk194;
	TESPackage*					pPackage198;
	float						fOneMinusRadResist;
	ContinuousBeamProjectile*	pContinuousBeamProjectile;
	UInt32						uiSpeechExpression;
	UInt32						uiEmotionValue;
	UInt32						eSitSleepState;
	bool						bIsTurret;
	bool						bForceHit;
	bool						byte1B2;
	bool						byte1B3;

	void EquipItem(TESForm* objType, UInt32 equipCount = 1, ExtraDataList* itemExtraList = NULL, UInt32 unk3 = 1, bool lockEquip = false, UInt32 unk5 = 1);	// unk3 apply enchantment on player differently
	void UnequipItem(TESForm* objType, UInt32 unequipCount = 1, ExtraDataList* itemExtraList = NULL, UInt32 unk3 = 1, bool lockEquip = false, UInt32 unk5 = 1);
	SInt32 GetCurrentAction() const;
	bool IsInCombat() const;
	bool IsInReloadAnim() const;
	bool IsWeaponDrawn() const {
		return pkBaseProcess && pkBaseProcess->IsWeaponOut();
	}
	bool IsMoving() const;
	bool IsAiming() const;
	bool HasExtraRefractionProperty();
	bool IsGhost() const;
	bool CastsShadows() const;
	bool GetIsUnconscious() const;

	UInt32 GetMovementFlags() const;

	bool GetPackageBehaviourFlag(UInt32 auiFlag) const;

	EquippedItemsList	GetEquippedItems();
	ExtraContainerDataArray	GetEquippedEntryDataList();
	ExtraContainerExtendDataArray GetEquippedExtendDataList();

	UInt32 GetCurrentWeaponID() {
		if (!pkBaseProcess) return 0;

		const ItemChange* pCurrentWeapon = pkBaseProcess->GetWeapon();
		if (!pCurrentWeapon || !pCurrentWeapon->pObject) return 0;

		return pCurrentWeapon->pObject->uiFormID;
	}

	TESObjectWEAP* GetCurrentWeapon() {
		if (pkBaseProcess) {
			const ItemChange* pCurrentWeapon = pkBaseProcess->GetWeapon();
			if (pCurrentWeapon)
				return reinterpret_cast<TESObjectWEAP*>(pCurrentWeapon->pObject);
		}
		return nullptr;
	}

	TESPackage* GetCurrentPackage() const;

	double GetDistanceToRef(TESObjectREFR* apTarget, bool abEnabledOnly, bool abSharedSpace) const;

	SInt32 GetDetected(bool a2, Actor* apTarget, bool& arbInLOS, bool abInCombat, bool a6, int unused, bool& a8);

	bool GetShouldAttack(Actor* apTarget, bool a3, SInt32& a4, bool a5);

	bool GetIsActorCombatTarget(Actor* apTarget) const;

	void StopMoving();

	bool IsDoingAttackAnimation() const;

	void SetMovementFlag(UInt16 ausMovementFlags);
	float GetMovementSpeed() const;
};

ASSERT_SIZE(Actor, 0x1B4);
ASSERT_OFFSET(Actor, pRagdollController, 0xAC);
ASSERT_OFFSET(Actor, pActorMover, 0x190);