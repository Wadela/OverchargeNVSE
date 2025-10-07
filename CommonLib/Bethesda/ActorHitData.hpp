#pragma once
#include "NiPoint3.hpp"
#include "ActorValue.hpp"

class Actor;
class Projectile;
class Explosion;
class TESObjectWEAP;

// 0x64
class ActorHitData 
{
public:
	enum EnumHitFlags
	{
		kFlag_TargetIsBlocking			= 1 << 0,
		kFlag_TargetWeaponOut			= 1 << 1,
		kFlag_IsCritical				= 1 << 2,
		kFlag_OnDeathCritEffect			= 1 << 3,
		kFlag_IsFatal					= 1 << 4,
		kFlag_DismemberLimb				= 1 << 5,
		kFlag_ExplodeLimb				= 1 << 6,
		kFlag_CrippleLimb				= 1 << 7,
		kFlag_BreakWeaponNonEmbedded	= 1 << 8,
		kFlag_BreakWeaponEmbedded		= 1 << 9,
		kFlag_IsSneakAttack				= 1 << 10,
		kFlag_ArmorPenetrated = 0x80000000	// JIP only
	};

	Actor*				pkSource;		// 00
	Actor*				pkTarget;		// 04
	union
	{
		Projectile*		pProjectile;
		Explosion*		pExplosion;
	};
	ActorValue::Index	eWeaponActorValue;	// 0C
	UInt32				eDamageLimb;
	float				fHealthDamage;
	float				fBaseWeaponDamage;
	float				fFatigueDamage;
	float				fLimbDamage;
	float				fBlockDTMod;
	float				fArmorDamage;
	float				fDamageToWeapon;
	TESObjectWEAP*		pWeapon;
	float				fHealthPercent;
	NiPoint3			kImpactPos;
	NiPoint3			kImpactAngle;
	SpellItem*			pCriticalEffect;
	UInt32				pVATSAction;
	Bitfield32			uiFlags;
	float				fDamageMult;
	UInt32				uiRefCount;
};
static_assert(sizeof(ActorHitData) == 0x64);