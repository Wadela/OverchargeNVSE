#pragma once

#include "ActorValue.hpp"

class Actor;

class ActorValueOwner {
public:
	ActorValueOwner();
	~ActorValueOwner();

	// GetBaseActorValue (used from Eval) result in EAX
	virtual UInt32	GetBaseActorValueI(ActorValue::Index aeIndex);
	// GetBaseActorValue internal, result in st
	virtual float	GetBaseActorValueF(ActorValue::Index aeIndex);
	// GetActorValue internal, result in EAX
	virtual UInt32	GetActorValueI(ActorValue::Index aeIndex);
	// GetActorValue (used from Eval) result in EAX
	virtual float	GetActorValueF(ActorValue::Index aeIndex);
	// GetBaseActorValue04 (internal) result in st
	virtual float	Fn_04(ActorValue::Index aeIndex);
	// GetDamageActorValue or GetModifiedActorValue
	// called from Fn_08, result in st, added to Fn_01
	virtual float	Fn_05(void* arg);
	virtual float	GetModifier(ActorValue::Index aeIndex);
	// Manipulate GetPermanentActorValue, maybe convert to integer.
	virtual UInt32	GetPermanentActorValueI(ActorValue::Index aeIndex);
	// GetPermanentActorValue (used from Eval) result in EAX
	virtual float	GetPermanentActorValueF(ActorValue::Index aeIndex);
	// GetActorBase (= this - 0x100) or GetActorBase (= this - 0x0A4)
	virtual Actor*	GetActor();
	virtual UInt16	GetLevel();

	// SkillsCurrentValue[14] at index 20
};

ASSERT_SIZE(ActorValueOwner, 0x4);