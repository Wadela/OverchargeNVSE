#pragma once

// Added to remove a cyclic dependency between GameForms.h and GameExtraData.h
#include "Utilities.h"

enum
{
	/*00*/kXData_ExtraUnknown00,
	/*01*/kXData_ExtraHavok,
	/*02*/kXData_ExtraCell3D,
	/*03*/kXData_ExtraCellWaterType,
	/*04*/kXData_ExtraRegionList,
	/*05*/kXData_ExtraSeenData,
	/*06*/kXData_ExtraEditorID,
	/*07*/kXData_ExtraCellMusicType,
	/*08*/kXData_ExtraCellClimate,
	/*09*/kXData_ExtraProcessMiddleLow,
	/*0A*/kXData_ExtraCellCanopyShadowMask,
	/*0B*/kXData_ExtraDetachTime,
	/*0C*/kXData_ExtraPersistentCell,
	/*0D*/kXData_ExtraScript,
	/*0E*/kXData_ExtraAction,
	/*0F*/kXData_ExtraStartingPosition,
	/*10*/kXData_ExtraAnim,
	/*11*/kXData_ExtraUnknown11,
	/*12*/kXData_ExtraUsedMarkers,
	/*13*/kXData_ExtraDistantData,
	/*14*/kXData_ExtraRagdollData,
	/*15*/kXData_ExtraContainerChanges,
	/*16*/kXData_ExtraWorn,
	/*17*/kXData_ExtraWornLeft,
	/*18*/kXData_ExtraPackageStartLocation,
	/*19*/kXData_ExtraPackage,
	/*1A*/kXData_ExtraTrespassPackage,
	/*1B*/kXData_ExtraRunOncePacks,
	/*1C*/kXData_ExtraReferencePointer,
	/*1D*/kXData_ExtraFollower,
	/*1E*/kXData_ExtraLevCreaModifier,
	/*1F*/kXData_ExtraGhost,
	/*20*/kXData_ExtraOriginalReference,
	/*21*/kXData_ExtraOwnership,
	/*22*/kXData_ExtraGlobal,
	/*23*/kXData_ExtraRank,
	/*24*/kXData_ExtraCount,
	/*25*/kXData_ExtraHealth,
	/*26*/kXData_ExtraUses,
	/*27*/kXData_ExtraTimeLeft,
	/*27*/kXData_ExtraJIP = kXData_ExtraTimeLeft,
	/*28*/kXData_ExtraCharge,
	/*29*/kXData_ExtraLight,
	/*2A*/kXData_ExtraLock,
	/*2B*/kXData_ExtraTeleport,
	/*2C*/kXData_ExtraMapMarker,
	/*2D*/kXData_ExtraUnknown2D,
	/*2E*/kXData_ExtraLeveledCreature,
	/*2F*/kXData_ExtraLeveledItem,
	/*30*/kXData_ExtraScale,
	/*31*/kXData_ExtraSeed,
	/*32*/kXData_ExtraNonActorMagicCaster,
	/*33*/kXData_ExtraNonActorMagicTarget,
	/*34*/kXData_ExtraUnknown34,
	/*35*/kXData_ExtraPlayerCrimeList,
	/*36*/kXData_ExtraUnknown36,
	/*37*/kXData_ExtraEnableStateParent,
	/*38*/kXData_ExtraEnableStateChildren,
	/*39*/kXData_ExtraItemDropper,
	/*3A*/kXData_ExtraDroppedItemList,
	/*3B*/kXData_ExtraRandomTeleportMarker,
	/*3C*/kXData_ExtraMerchantContainer,
	/*3D*/kXData_ExtraSavedHavokData,
	/*3E*/kXData_ExtraCannotWear,
	/*3F*/kXData_ExtraPoison,
	/*40*/kXData_ExtraUnknown40,
	/*41*/kXData_ExtraLastFinishedSequence,
	/*42*/kXData_ExtraSavedAnimation,
	/*43*/kXData_ExtraNorthRotation,
	/*44*/kXData_ExtraXTarget,
	/*45*/kXData_ExtraFriendHits,
	/*46*/kXData_ExtraHeadingTarget,
	/*47*/kXData_ExtraUnknown47,
	/*48*/kXData_ExtraRefractionProperty,
	/*49*/kXData_ExtraStartingWorldOrCell,
	/*4A*/kXData_ExtraHotkey,
	/*4B*/kXData_ExtraUnknown4B,
	/*4C*/kXData_ExtraEditorRefMovedData,
	/*4D*/kXData_ExtraInfoGeneralTopic,
	/*4E*/kXData_ExtraHasNoRumors,
	/*4F*/kXData_ExtraSound,
	/*50*/kXData_ExtraTerminalState,
	/*51*/kXData_ExtraLinkedRef,
	/*52*/kXData_ExtraLinkedRefChildren,
	/*53*/kXData_ExtraActivateRef,
	/*54*/kXData_ExtraActivateRefChildren,
	/*55*/kXData_ExtraTalkingActor,
	/*56*/kXData_ExtraObjectHealth,
	/*57*/kXData_ExtraDecalRefs,
	/*58*/kXData_ExtraUnknown58,
	/*59*/kXData_ExtraCellImageSpace,
	/*5A*/kXData_ExtraNavMeshPortal,
	/*5B*/kXData_ExtraModelSwap,
	/*5C*/kXData_ExtraRadius,
	/*5D*/kXData_ExtraRadiation,
	/*5E*/kXData_ExtraFactionChanges,
	/*5F*/kXData_ExtraDismemberedLimbs,
	/*60*/kXData_ExtraActorCause,
	/*61*/kXData_ExtraMultiBound,
	/*62*/kXData_ExtraMultiBoundData,
	/*63*/kXData_ExtraMultiBoundRef,
	/*64*/kXData_ExtraUnknown64,
	/*65*/kXData_ExtraReflectedRefs,
	/*66*/kXData_ExtraReflectorRefs,
	/*67*/kXData_ExtraEmittanceSource,
	/*68*/kXData_ExtraRadioData,
	/*69*/kXData_ExtraCombatStyle,
	/*6A*/kXData_ExtraUnknown6A,
	/*6B*/kXData_ExtraPrimitive,
	/*6C*/kXData_ExtraOpenCloseActivateRef,
	/*6D*/kXData_ExtraAnimNoteReciever,
	/*6E*/kXData_ExtraAmmo,
	/*6F*/kXData_ExtraPatrolRefData,
	/*70*/kXData_ExtraPackageData,
	/*71*/kXData_ExtraOcclusionPlane,
	/*72*/kXData_ExtraCollisionData,
	/*73*/kXData_ExtraSayTopicInfoOnceADay,
	/*74*/kXData_ExtraEncounterZone,
	/*75*/kXData_ExtraSayToTopicInfo,
	/*76*/kXData_ExtraOcclusionPlaneRefData,
	/*77*/kXData_ExtraPortalRefData,
	/*78*/kXData_ExtraPortal,
	/*79*/kXData_ExtraRoom,
	/*7A*/kXData_ExtraHealthPerc,
	/*7B*/kXData_ExtraRoomRefData,
	/*7C*/kXData_ExtraGuardedRefData,
	/*7D*/kXData_ExtraCreatureAwakeSound,
	/*7E*/kXData_ExtraWaterZoneMap,
	/*7F*/kXData_ExtraUnknown7F,
	/*80*/kXData_ExtraIgnoredBySandbox,
	/*81*/kXData_ExtraCellAcousticSpace,
	/*82*/kXData_ExtraReservedMarkers,
	/*83*/kXData_ExtraWeaponIdleSound,
	/*84*/kXData_ExtraWaterLightRefs,
	/*85*/kXData_ExtraLitWaterRefs,
	/*86*/kXData_ExtraWeaponAttackSound,
	/*87*/kXData_ExtraActivateLoopSound,
	/*88*/kXData_ExtraPatrolRefInUseData,
	/*89*/kXData_ExtraAshPileRef,
	/*8A*/kXData_ExtraCreatureMovementSound,
	/*8B*/kXData_ExtraFollowerSwimBreadcrumbs,
	/*8C*/kXData_ExtraCellImpactSwap,
	/*8D*/kXData_ExtraWeaponModFlags,
	/*8E*/kXData_ExtraModdingItem,
	/*8F*/kXData_ExtraSecuritronFace,
	/*90*/kXData_ExtraAudioMarker,
	/*91*/kXData_ExtraAudioBuoyMarker,
	/*92*/kXData_ExtraSpecialRenderFlags,
	kXData_ExtraMax
};

#define GetExtraType(xDataList, Type) (Type*)(xDataList)->GetByType(kXData_ ## Type)
#define CreateExtraType(Type) \
	UInt32 *dataPtr = (UInt32*)Game_HeapAlloc<Type>(); \
	dataPtr[0] = kVtbl_ ## Type; \
	dataPtr[1] = kXData_ ## Type; \
	dataPtr[2] = 0;

#define CreateExtraType(Type) \
	UInt32 *dataPtr = (UInt32*)Game_HeapAlloc<Type>(); \
	dataPtr[0] = kVtbl_ ## Type; \
	dataPtr[1] = kXData_ ## Type; \
	dataPtr[2] = 0;

class TESForm;

// C+?
class BSExtraData
{
public:
	BSExtraData();
	virtual ~BSExtraData();

	virtual bool Differs(BSExtraData* extra);	// 001

	static BSExtraData* Create(UInt8 xType, UInt32 size, UInt32 vtbl);

	//void		* _vtbl;	// 000
	UInt8		type;		// 004
	UInt8		pad[3];		// 005
	BSExtraData	* next;		// 008

};

// 020
struct BaseExtraList
{
	bool HasType(UInt32 type) const;

	//Same as Add
	__forceinline BSExtraData* AddExtra(BSExtraData* toAdd)
	{
		return ThisCall<BSExtraData*>(0x40FF60, this, toAdd); //0x40FF60 ADDR_AddExtraData
	}

	__forceinline BSExtraData *GetByType(UInt8 type) const
	{
		return ThisStdCall<BSExtraData*>(0x410220, this, type);
	}

	void MarkType(UInt32 type, bool bCleared);

	__forceinline void Remove(BSExtraData *toRemove, bool doFree)
	{
		ThisStdCall(0x410020, this, toRemove, doFree);
	}

	__forceinline void RemoveByType(UInt32 type)
	{
		ThisStdCall(0x410140, this, type);
	}

	__forceinline BSExtraData *Add(BSExtraData *toAdd)
	{
		return ThisStdCall<BSExtraData*>(0x40FF60, this, toAdd);
	}

	__forceinline void RemoveAll(bool doFree)
	{
		ThisStdCall(0x40FAE0, this, doFree);
	}

	bool MarkScriptEvent(UInt32 eventMask, TESForm* eventTarget);

	__forceinline void Copy(BaseExtraList *from)
	{
		ThisStdCall(0x411EC0, this, from);
	}

	//Vanilla copy from, this runs when dropping items. Regular copy works better? couldn't get this to copy kExtraData_Count
	__forceinline void CopyFrom(const BaseExtraList* sourceList, bool bCopyAndRemove)
	{
		ThisCall(0x412490, this, sourceList, bCopyAndRemove);
	}

	SInt32 GetCount() const;

	void DebugDump() const;

	bool IsWorn();

	void		** m_vtbl;					// 000
	BSExtraData	* m_data;					// 004
	UInt8		m_presenceBitfield[0x15];	// 008 - if a bit is set, then the extralist should contain that extradata
											// bits are numbered starting from the lsb
	UInt8		pad1D[3];					// 01D
};

class ExtraCount;

struct ExtraDataList : public BaseExtraList
{
	ExtraDataList* CreateCopy(bool bCopyAndRemove = false);
	static ExtraDataList * Create(BSExtraData* xBSData = NULL);
	ExtraCount* AddExtraCount(SInt32 count);

	bool ContainsMatch(ExtraDataList* xCompare, bool doFree = false);
	bool ContainsMatch(BSExtraData* xCompare, bool doFree = false);

	//Generally used for making new items using another items extra data list. Does not copy data like worn extra data.
	static ExtraDataList* CopyItemData(ExtraDataList* xCopyFrom, bool doFree, ExtraDataList* xCopyTo = nullptr);

};

STATIC_ASSERT(offsetof(BaseExtraList, m_presenceBitfield) == 0x008);
STATIC_ASSERT(sizeof(ExtraDataList) == 0x020);

