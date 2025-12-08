#pragma once
#include "MobileObject.hpp"
#include "NiTPointerList.hpp"

class NiPointLight;
class NonActorMagicCaster;
class BSSoundHandle;
class bhkSimpleShapePhantom;

class Explosion : public MobileObject
{
public:
	Explosion();
	~Explosion();


	float								fAge;
	float								f3DLifetime;
	float								fInnerRadius;
	float								fOuterRadius;
	float								fImageSpaceRadius;
	float								fDamageMult;
	NiPointer<bhkSimpleShapePhantom>	spPhantom0A0;
	BSSimpleList<void*>					kTargets;
	BSSoundHandle*						pSoundHandles[6];
	NiPointer<NiPointLight>				spLight;
	TESObjectREFR*						pOwnerRef;
	TESObjectREFR*						pExplodedRef;
	NiPointer<ActorCause>				spActorCause;
	bool								bDecalsPlaced;
	bool								bTargetsFound;
	bool								bTargetsProcessed;
	bool								bForcesApplied;
	bool								bSkipSwap;
	bool								bUnderwater;
	NonActorMagicCaster*				pCaster;
	int									gapE0;
	int									iFrameCount;
	NiPoint3							kClosestPoint;
	NiPoint3							kClosestPointNormal;
	float								fCalculatedDamage;


  static NiTPointerList<TESObjectREFR*>* GetWaterReflectedRefs();
};
static_assert(sizeof(Explosion) == 0x104);