#pragma once

#include "GameForms.h"
#include "GameBSExtraData.h"
#include "GameExtraData.h"
#include "GameProcess.h"
#include "NiPoint.h"

//From JIP
#define ADDR_AddExtraData		0x40FF60
#define ADDR_RemoveExtraType	0x410140
#define ADDR_LoadModel			0x447080
#define ADDR_ApplyAmmoEffects	0x59A030
#define ADDR_MoveToMarker		0x5CCB20
#define ADDR_ApplyPerkModifiers	0x5E58F0
#define ADDR_ReturnThis			0x6815C0
#define ADDR_PurgeTerminalModel	0x7FFE00
#define ADDR_EquipItem			0x88C650
#define ADDR_UnequipItem		0x88C790
#define ADDR_ReturnTrue			0x8D0360
#define ADDR_TileGetFloat		0xA011B0
#define ADDR_TileSetFloat		0xA012D0
#define ADDR_TileSetString		0xA01350
#define ADDR_InitFontInfo		0xA12020
#define ADDR_GetRandomInt		0xAA5230
#define ADDR_ReturnThis2		0xE68810

#define EXTRA_DATA_CS			0x11C3920
#define SCENE_LIGHTS_CS			0x11F9EA0

#define HEX(a) std::bit_cast<UInt32>(a)

#define IS_REFERENCE(form) ((*(UInt32**)form)[0xF0 >> 2] == ADDR_ReturnTrue)
#define NOT_REFERENCE(form) ((*(UInt32**)form)[0xF0 >> 2] != ADDR_ReturnTrue)
#define IS_ACTOR(form) ((*(UInt32**)form)[0x100 >> 2] == ADDR_ReturnTrue)
#define NOT_ACTOR(form) ((*(UInt32**)form)[0x100 >> 2] != ADDR_ReturnTrue)
#define IS_PROJECTILE(form) ((*(UInt32**)form)[0x224 >> 2] == ADDR_ReturnTrue)
#define IS_NODE(object) ((*(UInt32**)object)[0xC >> 2] == ADDR_ReturnThis)
#define IS_GEOMETRY(object) ((*(UInt32**)object)[0x18 >> 2] == ADDR_ReturnThis2)

class TESObjectCELL;
struct ScriptEventList;
class ActiveEffect;
class NiNode;
class Animation;
struct InventoryRef;
struct NiPoint3;

// 008
class TESChildCell
{
public:
	TESChildCell();
	~TESChildCell();

	// no virtual destructor
	virtual TESObjectCELL *	GetPersistentCell(void);		// 000

//	void	** vtbl;	// 00
};

// 68
class TESObjectREFR : public TESForm
{
public:
	MEMBER_FN_PREFIX(TESObjectREFR);

	TESObjectREFR();
	~TESObjectREFR();

	/*138*/virtual bool		GetStartingPosition(NiVector3* outPos, NiVector3* outRot, TESForm** outWrldOrCell, TESForm* defaultWrldOrCell);
	/*13C*/virtual void		SayTopic(TESSound* sound, TESTopic* topic, TESObjectREFR* target, bool dontUseNiNode, bool notVoice, bool useLipFile, UInt8 unused, bool subtitles);
	virtual void		Unk_50(void);
	/*144*/virtual void		DamageObject(float damage, bool allowDestroyed);
	/*148*/virtual bool		GetCastsShadows();
	/*14C*/virtual void		SetCastsShadows(bool doSet);
	virtual void		Unk_54(void);
	virtual void		Unk_55(void);
	virtual void		Unk_56(void);
	/*15C*/virtual bool		IsObstacle();
	/*160*/virtual bool		BaseIsQuestItem();
	virtual void		Unk_59(void);
	virtual void		Unk_5A(void);
	virtual void		Unk_5B(void);
	virtual void		Unk_5C(void);
	virtual void		Unk_5D(void);
	/*178*/virtual void		RefreshDynamicLight();
	/*17C*/virtual TESObjectREFR* RemoveItem(TESForm* toRemove, BaseExtraList* extraList, UInt32 quantity, bool keepOwner, bool drop, TESObjectREFR* destRef, UInt32 unk6, UInt32 unk7, bool unk8, bool unk9);
	/*180*/virtual void		Unk_60(void);
	/*184*/virtual bool		LoadEquipedItem3D(TESForm* item, UInt32 count, ExtraDataList* xData, bool lockEquip);
	/*188*/virtual void		Unk_62(void);
	/*18C*/virtual void		Unk_63(void);
	/*190*/virtual void		AddItem(TESForm* item, ExtraDataList* xDataList, UInt32 quantity);
	virtual void		Unk_65(void);
	virtual void		Unk_66(void);
	virtual void		Unk_67(void);					// Actor: GetMagicEffectList
	virtual bool		GetIsChildSize(bool checkHeight);		// 068 Actor: GetIsChildSize
	virtual UInt32		GetActorUnk0148(void);			// result can be interchanged with baseForm, so TESForm* ?
	virtual void		SetActorUnk0148(UInt32 arg0);
	virtual void		Unk_6B(void);
	virtual void		Unk_6C(void);	// REFR: GetBSFaceGenNiNodeSkinned
	virtual void		Unk_6D(void);	// REFR: calls 006C
	virtual void		Unk_6E(void);	// MobileActor: calls 006D then NiNode::Func0040
	virtual void		Unk_6F(void);
	virtual void		Unk_70(void);
	virtual void		AnimateNiNode(void);					// same in FOSE ! identical to Func0052 in OBSE which says (inits animation-related data, and more)
	virtual void		GenerateNiNode(bool arg0);				// same in FOSE !
	virtual void		Set3D(NiNode* niNode, bool unloadArt);	// same in FOSE !
	virtual NiNode *	GetNiNode(void);						// Same in FOSE !
	virtual void		Unk_75(void);
	virtual void		Unk_76(void);
	virtual void		Unk_77(void);
	virtual void		Unk_78(void);
	virtual AnimData*	GetAnimData(void);			// 0079
	virtual ValidBip01Names * GetValidBip01Names(void);	// 007A	Character only
	virtual ValidBip01Names * CallGetValidBip01Names(void);
	virtual void		SetValidBip01Names(ValidBip01Names validBip01Names);
	/*1F4*/virtual NiVector3* GetPos() const;				// GetPos or GetDistance
	/*1F8*/virtual void		Unk_7E(UInt32 arg0);
	/*1FC*/virtual void		Unk_7F(void);
	/*200*/virtual void		Unk_80(UInt32 arg0);
	/*204*/virtual void		Unk_81(UInt32 arg0);
	/*208*/virtual void		Unk_82(void);
	/*20C*/virtual NiNode* GetProjectileNode();
	/*210*/virtual void		SetProjectileNode(NiNode* node);
	/*214*/virtual UInt32	GetSitSleepState();
	/*218*/virtual bool		IsCharacter();			// return false for Actor and Creature, true for character and PlayerCharacter
	/*21C*/virtual bool		IsCreature();
	/*220*/virtual bool		IsExplosion();
	/*224*/virtual bool		IsProjectile() const;
	/*228*/virtual void		SetParentCell(TESObjectCELL* cell);			// SetParentCell (Interior only ?)
	/*22C*/virtual bool		HasHealth(bool arg0);	// HasHealth (baseForm health > 0 or Flags bit23 set)
	/*230*/virtual bool		GetHasKnockedState();
	/*234*/virtual bool		GetIsParalyzed();
	/*238*/virtual void		Unk_8E(void);
	/*23C*/virtual void		Unk_8F(void);
	/*240*/virtual void		MoveToHighProcess();

	enum {
		kFlags_Unk00000002	= 0x00000002,
		kFlags_Deleted		= 0x00000020,		// refr removed from .esp or savegame
		kFlags_Persistent	= 0x00000400,		//shared bit with kFormFlags_QuestItem
		kFlags_Temporary	= 0x00004000,
		kFlags_Taken		= kFlags_Deleted | kFlags_Unk00000002,

		kChanged_Inventory	= 0x08000000,
	};

	struct RenderState
	{
		TESObjectREFR* currWaterRef;		// 00
		UInt32			underwaterCount;	// 04	0-0x13 when fully-underwater; exterior only
		float			waterLevel;			// 08
		float			revealDistance;		// 0C
		UInt32			flags;				// 10
		NiNode*			rootNode;			// 14
	};

	struct EditorData {
		UInt32	unk00;	// 00
	};
	// 0C

#ifdef EDITOR
	EditorData	editorData;			// +04
#endif

	TESChildCell	childCell;				// 018

	UInt32			unk1C;					// 01C

	TESForm			* baseForm;				// 020
	
	float			rotX, rotY, rotZ;		// 024 - either public or accessed via simple inline accessor common to all child classes
	float			posX, posY, posZ;		// 030 - seems to be private
	float			scale;					// 03C 

	TESObjectCELL	* parentCell;			// 040
	ExtraDataList	extraDataList;			// 044
	RenderState		* renderState;			// 064	- (05C in FOSE)

	//Copied from JIP LN NVSE - Needed for Overcharge NVSE. Formerly known as GetNiNode()
	NiNode* GetNiNodeJIP() const; 

	ExtraScript* GetExtraScript() const;
	ScriptEventList *	GetEventList() const;

	__forceinline void SetAngleOnReference(float afX, float afY, float afZ)
	{
		ThisStdCall(0x0575700, this, afX, afY, afZ);
	}

	__forceinline void SetLocationOnReference(NiPoint3* pos)
	{
		ThisStdCall(0x0575830, this, pos);
	}

	__forceinline void MoveTo(NiPoint3* pos)
	{
		ThisStdCall(0x0575830, this, pos);
	}

	__forceinline int SetDroppedRef(bool set) {
		return ThisStdCall(0x0564C60, this, set);
	}

	__forceinline void SetRefPersists(bool set) {
		ThisStdCall(0x0565480, this, set);
	}

	__forceinline TESObjectREFR* PlaceAtMe(TESForm* form, UInt32 count, UInt32 distance, UInt32 direction, float health)
	{
		return CdeclCall<TESObjectREFR*>(0x5C4B30, this, form, count, distance, direction, health);
	}

	Instance* GetWeaponBase();
	UInt8 GetWeaponModFlags();
	void SetWeaponModFlags(UInt8);
	TESForm* GetLocation();

	//TESObjectREFR* PlaceAtMe(TESObjectREFR* refr, TESForm* form, UInt32 count, UInt32 distance, UInt32 direction, float health);

	bool IsTaken() const { return (flags & kFlags_Taken) != 0; }
	bool IsPersistent() const { return (flags & kFlags_Persistent) != 0; }
	bool IsTemporary() { return (flags & kFlags_Temporary) ? true : false; }
	bool IsDeleted() const { return (flags & kFlags_Deleted) ? true : false; }

	__forceinline NiNode* GetRefNiNode() const { return renderState ? renderState->rootNode : nullptr; }

	bool Update3D();
	bool Update3D_v1c();	// Less worse version as used by some modders
	TESContainer* GetContainer();
	bool IsMapMarker();

	void DeleteReference();
	bool GetDisabled() const;
	ContChangesEntryList* GetContainerChangesList() const;
	ContChangesEntry* GetContainerChangesEntry(TESForm* itemForm) const;

	SInt32 GetItemCount(TESForm* form) const;
	void AddItemAlt(TESForm* item, UInt32 count, float condition, UInt32 doEquip = 0, UInt32 noMessage = 1);
	bool GetInventoryItems(InventoryItemsMap &invItems);
	ExtraDroppedItemList* GetDroppedItems();

	double GetHeadingAngle(const TESObjectREFR* to) const;
	bool __fastcall GetInSameCellOrWorld(TESObjectREFR* target) const;
	NiAVObject* __fastcall GetNifBlock(TESObjectREFR* thisObj, UInt32 pcNode, const char* blockName);
	float __vectorcall GetDistance(TESObjectREFR* target) const;

	static TESObjectREFR* __stdcall Create(bool bTemp = false);

	NiNode* __fastcall GetNode(const char* nodeName) const;

	//Returns static baseObject
	TESForm* GetBaseObject() const;
	std::pair<TESAmmo*, UInt32> FindAmmoType(TESObjectWEAP* weapon);
	TESAmmo* CycleAmmoType(TESObjectWEAP* weapon, TESAmmo* ammo);

	MEMBER_FN_PREFIX(TESObjectREFR);
#if RUNTIME
	DEFINE_MEMBER_FN(Activate, bool, 0x00573170, TESObjectREFR*, UInt32, UInt32, UInt32);	// Usage Activate(actionRef, 0, 0, 1); found inside Cmd_Activate_Execute as the last call (190 bytes)
	DEFINE_MEMBER_FN(Set3D, void, 0x0094EB40, NiNode*, bool);	// Usage Set3D(niNode, true); virtual func 0073
#endif
};

TESForm* GetPermanentBaseForm(TESObjectREFR* thisObj);	// For LevelledForm, find real baseForm, not temporary one.

// Taken from JIP LN NVSE.
float __vectorcall GetDistance3D(const TESObjectREFR* ref1, const TESObjectREFR* ref2);

STATIC_ASSERT(offsetof(TESObjectREFR, baseForm) == 0x020);
STATIC_ASSERT(offsetof(TESObjectREFR, extraDataList) == 0x044);
STATIC_ASSERT(sizeof(TESObjectREFR) == 0x068);

class BaseProcess;

// 088
class MobileObject : public TESObjectREFR
{
public:
	MobileObject();
	~MobileObject();

	virtual void		StartHighProcess(void);		// 090
	virtual void		Unk_91(void);
	virtual void		Unk_92(void);
	virtual void		Unk_93(void);
	virtual void		Unk_94(void);
	virtual void		Unk_95(void);
	virtual void		Unk_96(void);
	virtual void		Unk_97(void);
	virtual void		Unk_98(void);
	virtual void		Unk_99(void);
	virtual void		Unk_9A(void);
	virtual void		Unk_9B(void);
	virtual void		Unk_9C(void);
	virtual void		Unk_9D(void);
	virtual void		Unk_9E(void);
	virtual void		Unk_9F(void);
	virtual void		Unk_A0(void);	// StartConversation(targetActor, subjectLocationData, targetLocationData, headTrack, allowMovement, arg, topicID, arg, arg
	virtual void		Unk_A1(void);
	virtual void		Unk_A2(void);
	virtual void		Unk_A3(void);
	virtual void		Unk_A4(void);
	virtual void		Unk_A5(void);
	virtual void		Unk_A6(void);
	virtual void		Unk_A7(void);
	virtual void		Unk_A8(void);
	virtual void		Unk_A9(void);
	virtual void		Unk_AA(void);
	virtual void		Unk_AB(void);
	virtual void		Unk_AC(void);
	virtual void		Unk_AD(void);
	virtual void		Unk_AE(void);
	virtual float		AdjustRot(UInt32 arg1);
	virtual void		Unk_B0(void);
	virtual void		Unk_B1(void);
	virtual void		Unk_B2(void);
	virtual void		Unk_B3(void);
	virtual void		Unk_B4(void);
	virtual void		Unk_B5(void);
	virtual void		Unk_B6(void);
	virtual void		Unk_B7(void);
	virtual void		Unk_B8(void);
	virtual void		Unk_B9(void);
	virtual void		Unk_BA(void);
	virtual void		Unk_BB(void);
	virtual void		Unk_BC(void);
	virtual void		Unk_BD(void);
	virtual void		Unk_BE(void);
	virtual void		Unk_BF(void);
	virtual void		Unk_C0(void);
	
	BaseProcess	* baseProcess;	// 068
	UInt32		unk06C;			// 06C - loaded	set to the talking actor ref when initialising ExtraTalkingActor
	UInt32		unk070;			// 070 - loaded
	UInt32		unk074;			// 074 - loaded
	UInt32		unk078;			// 078 - loaded
	UInt8		unk07C;			// 07C - loaded
	UInt8		unk07D;			// 07D - loaded
	UInt8		unk07E;			// 07E - loaded
	UInt8		unk07F;			// 07F - loaded
	UInt8		unk080;			// 080 - loaded
	UInt8		unk081;			// 081 - loaded
	UInt8		unk082;			// 082 - cleared when starting combat on player
	UInt8		unk083;			// 083 - loaded
	UInt8		unk084;			// 084 - loaded
	UInt8		unk085;			// 085 - loaded
	UInt8		unk086;			// 086 - loaded
	UInt8		unk087;			// 087	Init'd to the inverse of NoLowLevelProcessing
};
STATIC_ASSERT(offsetof(MobileObject, baseProcess) == 0x068);
STATIC_ASSERT(sizeof(MobileObject) == 0x088);

// 00C
class MagicCaster
{
public:
	MagicCaster();
	~MagicCaster();

	UInt32 vtabl;
	UInt32 unk004;	// 004
	UInt32 unk008;	// 008
};

STATIC_ASSERT(sizeof(MagicCaster) == 0x00C);

typedef tList<ActiveEffect> EffectNode;
// 010
class MagicTarget
{
public:
	MagicTarget();
	~MagicTarget();

	virtual void	Destructor(void);
	virtual TESObjectREFR *	GetParent(void);
	virtual EffectNode *	GetEffectList(void);

	UInt8			byt004;		// 004 
	UInt8			byt005;		// 005 
	UInt8			byt006[2];	// 006-7
	tList<void*>	lst008;		// 008

	void RemoveEffect(EffectItem *effItem);

	void StopEffect(void *arg0, bool arg1)
	{
		ThisStdCall(0x8248E0, this, arg0, arg1);
	}
};

STATIC_ASSERT(sizeof(MagicTarget) == 0x010);

class hkaRaycastInterface
{
public:
	hkaRaycastInterface();
	~hkaRaycastInterface();
	virtual hkaRaycastInterface*	Destroy(bool doFree);
	virtual void					Unk_01(void* arg0);
	virtual void					Unk_02(void);

	// Assumed to be 0x010 bytes due to context where the vftable is used
	UInt32	unk000[(0x010 - 0x004) >> 2];	// 0000
};
STATIC_ASSERT(sizeof(hkaRaycastInterface) == 0x010);

class bhkRagdollController : public hkaRaycastInterface
{
public:
	bhkRagdollController();
	~bhkRagdollController();

	UInt32	unk000[(0x021C - 0x010) >> 2];	// 0010
	UInt8	fill021C[3];					// 021C
	bool	bool021F;						// 021F	when null assume FIK status is false
	bool	fikStatus;						// 0220
	UInt8	fill0221[3];					// 0221
};
STATIC_ASSERT(sizeof(bhkRagdollController) == 0x0224);

class bhkRagdollPenetrationUtil;
class ActorMover;
class PlayerMover;
class ImageSpaceModifierInstanceDOF;
class ImageSpaceModifierInstanceDRB;

class ActorMover	// I need to call Func008
{
public:
	virtual void		Destroy(void);
	virtual void		SetMovementFlag(UInt32 flags);
	virtual void		ClearMovementFlag(void);
	virtual void		Unk_03(void);
	virtual void		Unk_04(void);
	virtual void		Unk_05(void);
	virtual void		Unk_06(void);
	virtual void		Unk_07(void);
	virtual UInt32		GetMovementFlags(void);	// for PlayerMover, it is GetMovementFlags
		// bit 11 = swimming 
		// bit 9 = sneaking
		// bit 8 = run
		// bit 7 = walk
		// bit 0 = keep moving (Q)
};

typedef std::vector<TESForm*> EquippedItemsList;

class Actor : public MobileObject
{
public:
	Actor();
	~Actor();
	
	/*304*/virtual UInt8	IsGuard();
	/*308*/virtual void		SetGuard(UInt8 setTo);
	/*30C*/virtual void		Unk_C3(void);
	/*310*/virtual void		Unk_C4(void);
	/*314*/virtual void		Unk_C5(void);
	/*318*/virtual void		Unk_C6(void);
	/*31C*/virtual void		SetIgnoreCrime(bool ignoreCrime);
	/*320*/virtual bool		GetIgnoreCrime();
	/*324*/virtual void		Resurrect(UInt8 arg1, UInt8 arg2, UInt8 arg3);
	/*328*/virtual void		Unk_CA(void);
	/*32C*/virtual void		Unk_CB(void);
	/*330*/virtual void		Unk_CC(void);
	/*334*/virtual void		Unk_CD(void);
	/*338*/virtual void		DamageHealthAndFatigue(float healthDmg, float fatigueDmg, Actor* source);
	/*33C*/virtual void		DamageActionPoints(float amount); // checks GetIsGodMode before decreasing
	/*340*/virtual void		Unk_D0(void);
	/*344*/virtual void		Unk_D1(void);
	/*348*/virtual void		UpdateMovement(float arg1, UInt32 arg2);
	/*34C*/virtual void		Unk_D3(void);
	/*350*/virtual void		Unk_D4(void);
	/*354*/virtual float	GetDefaultTurningSpeed();
	/*358*/virtual bool		IsOverencumbered();
	/*35C*/virtual void		Unk_D7(void);
	/*360*/virtual bool		IsPlayerRef();
	/*364*/virtual void		Unk_D9(void);
	/*368*/virtual void		Unk_DA(void);
	/*36C*/virtual void		Unk_DB(void);
	/*370*/virtual void		Unk_DC(void);
	/*374*/virtual void		Unk_DD(void);
	/*378*/virtual void		Unk_DE(void);
	/*37C*/virtual TESRace* GetRace();
	/*380*/virtual float	GetHandReachTimesCombatDistance();
	/*384*/virtual void		Unk_E1(void);
	/*388*/virtual void		Unk_E2(void);
	/*38C*/virtual bool		IsPushable();
	/*390*/virtual UInt32	GetActorType();	// Creature = 0, Character = 1, PlayerCharacter = 2
	/*394*/virtual void		SetActorValue(UInt32 avCode, float value);
	/*398*/virtual void		SetActorValueInt(UInt32 avCode, UInt32 value);
	/*39C*/virtual void		ModActorValue(UInt32 avCode, float modifier, Actor* attacker);
	/*3A0*/virtual void		Unk_E8(void);
	/*3A4*/virtual void		ForceActorValue(UInt32 avCode, float modifier, UInt32 arg3);
	/*3A8*/virtual void		ModActorValueInt(UInt32 avCode, int modifier, UInt32 arg3);
	/*3AC*/virtual void		DamageActorValue(UInt32 avCode, float damage, Actor* attacker);
	/*3B0*/virtual void		Unk_EC(void);
	/*3B4*/virtual void		Unk_ED(void);
	/*3B8*/virtual void		Unk_EE(void);
	/*3BC*/virtual ContChangesEntry* GetPreferedWeapon(UInt32 unk);
	/*3C0*/virtual void		Unk_F0(void);
	/*3C4*/virtual void		ResetArmorDRDT();
	/*3C8*/virtual bool		DamageItem(ContChangesEntry* itemEntry, float damage, int unused);
	/*3CC*/virtual void		DropItem(TESForm* itemForm, ExtraDataList* xDataList, SInt32 count, NiVector3* pos, int arg5);
	/*3D0*/virtual void		DoActivate(TESObjectREFR* activatedRef, UInt32 count, bool arg3);
	/*3D4*/virtual void		Unk_F5(void);
	/*3D8*/virtual void		Unk_F6(void);
	/*3DC*/virtual void		Unk_F7(void);
	/*3E0*/virtual bool		AddActorEffect(SpellItem* actorEffect);
	/*3E4*/virtual bool		RemoveActorEffect(SpellItem* actorEffect);
	/*3E8*/virtual void		Reload(TESObjectWEAP* weapon, UInt32 animType, UInt8 hasExtendedClip);
	/*3EC*/virtual void		Reload2(TESObjectWEAP* weapon, UInt32 animType, UInt8 hasExtendedClip, UInt8 isInstantSwapHotkey);
	/*3F0*/virtual void		DecreaseAmmo();
	/*3F4*/virtual void		Unk_FD(void);
	virtual void		Unk_FE(void);
	virtual void		Unk_FF(void);
	virtual void		Unk_100(void);
	virtual void		Unk_101(void);
	virtual void		Unk_102(void);
	virtual void		Unk_103(void);
	virtual void		Unk_104(void);
	virtual void		Unk_105(void);
	virtual void		Unk_106(void);
	virtual void		Unk_107(void);
	virtual void		Unk_108(void);
	virtual void		Unk_109(void);
	virtual bool		IsInCombat(void);
	virtual Actor		*GetCombatTarget(void);
	virtual void		Unk_10C(void);
	virtual void		Unk_10D(void);
	virtual void		Unk_10E(void);
	virtual void		Unk_10F(void);
	virtual void		Unk_110(void);
	virtual void		Unk_111(void);
	virtual void		Unk_112(void);
	virtual void		Unk_113(void);
	virtual void		Unk_114(void);
	virtual void		Unk_115(void);
	virtual void		Unk_116(void);
	virtual void		Unk_117(void);
	virtual void		Unk_118(void);
	virtual void		Unk_119(void);
	virtual void		Unk_11A(void);
	virtual void		Unk_11B(void);
	virtual void		Unk_11C(void);
	virtual void		Unk_11D(void);
	virtual void		Unk_11E(void);
	virtual void		Unk_11F(void);
	virtual void		Unk_120(void);
	virtual void		Unk_121(void);
	virtual void		Unk_122(void);	//	0122	Check for LevelUp
	virtual void		Unk_123(void);
	virtual void		Unk_124(void);
	virtual void		Unk_125(void);
	virtual void		SetPerkRank(BGSPerk* perk, UInt8 rank, bool alt);
	virtual void		Unk_127(void);
	virtual UInt8		GetPerkRank(BGSPerk* perk, bool alt);
	virtual void		Unk_129(void);
	virtual void		Unk_12A(void);
	virtual void		Unk_12B(void);
	virtual void		Unk_12C(void);
	virtual void		Unk_12D(void);
	virtual void		Unk_12E(void);
	virtual void		Unk_12F(void);
	virtual void		Unk_130(void);
	virtual void		Unk_131(void);
	virtual void		Unk_132(void);
	virtual void		Unk_133(void);
	virtual void		Unk_134(void);
	virtual void		Unk_135(void);
	virtual void		Unk_136(void);
	void				FireWeapon();
	
	MagicCaster			magicCaster;			// 088
	MagicTarget			magicTarget;			// 094
	ActorValueOwner		avOwner;				// 0A4
	CachedValuesOwner	cvOwner;				// 0A8

	bhkRagdollController				* ragDollController;			// 0AC
	bhkRagdollPenetrationUtil			* ragDollPentrationUtil;		// 0B0
	UInt32								unk0B4[(0x104-0x0b4) >> 2];		// 0B4
	UInt8								unk104;							// 104 Is in combat
	UInt8								pad105[3];						// 105
	UInt32								lifeState;						// 108 saved as byte HasHealth = 1 or 2, optionally 6
	UInt32 unk10C;
	UInt32 nextAttackAnimGroupId110;
	UInt32								unk114[(0x140 - 0x114) >> 2];		// 10B 12C is an array (of combat targets ?)
	UInt32								unk140;							// 140 looks like a flag. Bit31 used while checking if non essential can take damage
	UInt8                               unk144;							// 144
	UInt8                               unk145;							// 145 Has to do with package evaluation
	UInt8                               unk146;							// 146 Has to do with package evaluation
	UInt8                               unk147;							// 147
	UInt32								unk148;							// 148
	UInt8								unk14C;							// 14C
	UInt8								isSwimming;						// 14D
	UInt8								unk14E;							// 14E
	UInt8								unk14F;							// 14F
	UInt32								unk150[(0x190-0x150) >> 2];		// 150	-	if Actor and not player: 0160 receives a copy of Pos[X,Y,Z] and Rot if 0170 is null during Func011B. 0170 receive ParentCell if Interior, Worldspace otherwize
	ActorMover							* actorMover;					// 190

	UInt32								unk194[(0x1B0-0x194) >> 2];		// 184 Byt018D = PlayerTeammate, 1AC is a list (or not! I have it loaded from a byte)

	UInt8								unk1B0;							// 1B0
	UInt8								unk1B1;							// 1B1
	UInt8								unk1B2[2];						// 1B2

	// OBSE: unk1 looks like quantity, usu. 1; ignored for ammo (equips entire stack). In NVSE, pretty much always forced internally to 1 
	// OBSE: itemExtraList is NULL as the container changes entry is not resolved before the call
	// NVSE: Default values are those used by the vanilla script functions.
	void EquipItem(TESForm * objType, UInt32 equipCount = 1, ExtraDataList* itemExtraList = NULL, UInt32 unk3 = 1, bool lockEquip = false, UInt32 playSound = 1);	// unk3 apply enchantment on player differently
	void UnequipItem(TESForm* objType, UInt32 unequipCount = 1, ExtraDataList* itemExtraList = NULL, UInt32 unk3 = 1, bool lockEquip = false, UInt32 playSound = 1);

	EquippedItemsList	GetEquippedItems();
	void EquipItemAlt(ContChangesEntry* entry, UInt32 noUnequip = 0, UInt32 noMessage = 1, bool playsound = 0);
	ExtraContainerDataArray	GetEquippedEntryDataList();
	ExtraContainerExtendDataArray GetEquippedExtendDataList();

	//Actor_Ext:

	bool SilentUnequip(TESForm* item, ExtraDataList* xData);
	bool SilentEquip(TESForm* item, ExtraDataList* xData);
	bool SilentEquip(ContChangesEntry* entry);
	
	bool QueueToSkipGroup(UInt16 AnimGroupID);	//Skips the next animation matching the AnimGroupID.
	//bool QueueToSkipAnimation();				//Skips the animation that's currently running in gamemode.

	TESObjectREFR* ReplaceInvObject(TESForm* form, InventoryRef* replace, UInt32 count, bool copy);

	ContChangesEntry* GetEquippedWeaponInfo() const;
	TESObjectWEAP* GetEquippedWeapon() const;
	
	//kNVSE
	bool IsSwimming() { return (GetMovementFlags() >> 11) & 1; }
	bool IsAnimActionReload() const;
	UInt32 GetMovementFlags() { return actorMover->GetMovementFlags(); }	// 11: IsSwimming, 9: IsSneaking, 8: IsRunning, 7: IsWalking, 0: keep moving

	// lStewieAl
	void SetWantsWeaponOut(bool wantsWeaponOut);
	void RefreshAnimData();
};

STATIC_ASSERT(offsetof(Actor, magicCaster) == 0x088);

class Character : public Actor
{
public:
	Character();
	~Character();

	virtual void		Unk_137(void);
	virtual void		Unk_138(void);

	ValidBip01Names	* validBip01Names;	// 1B4
	float			flt1B8;				// 1B8
	float			flt1BC;				// 1BC
	UInt8			byt1C0;				// 1C0
	UInt8			byt1C1;				// 1C1
	UInt16			unk1C2;				// 1C2
	float			flt1C4;				// 1C4
};

struct CombatActors;

typedef tList<BGSQuestObjective::Target> QuestObjectiveTargets;

// 9BC
class PlayerCharacter : public Character
{
public:
	PlayerCharacter();
	~PlayerCharacter();

	// used to flag controls as disabled in disabledControlFlags
	enum {
		kControlFlag_Movement		= 1 << 0,
		kControlFlag_Look			= 1 << 1,
		kControlFlag_Pipboy			= 1 << 2,
		kControlFlag_Fight			= 1 << 3,
		kControlFlag_POVSwitch		= 1 << 4,
		kControlFlag_RolloverText	= 1 << 5,
		kControlFlag_Sneak			= 1 << 6,
	};

	virtual void		Unk_139(void);
	virtual void		Unk_13A(void);

	// lotsa data

	UInt32								unk1C8[(0x244-0x1C8) >> 2];		// 1C8	0224 is a package of type 1C, 208 could be a DialogPackage, 206 questObjectiveTargets is valid
	float								unk244[0x4D];					// 244	have to be a set of ActorValue
	float								unk378[0x4D];					// 378	have to be a set of ActorValue
	UInt32								unk4AC;							// 4AC
	float								unk4B0[0x4D];					// 4B0	have to be a set of ActorValue
	BGSNote								* note;							// 5E4
	UInt32								unk574;							// 5E8
	ImageSpaceModifierInstanceDOF		* unk5EC;						// 5EC
	ImageSpaceModifierInstanceDOF		* unk5F0;						// 5F0
	ImageSpaceModifierInstanceDRB		* unk5F4;						// 5F4
	UInt32								unk5F8;							// 5F8
	tList<Actor>						teammates;						// 5FC
	UInt32								unk604[(0x648 - 0x604) >> 2];	// 604
	UInt8								unk648;							// 648
	UInt8								unk649;							// 649
	bool								unk64A;							// 64A	= not FirstPerson
	UInt8								unk64B;							// 64B
	bool								bThirdPerson;					// 64C
	UInt8								unk64D[3];	
	UInt32								unk650[(0x680 - 0x650) >> 2];	// 650 
	UInt8								disabledControlFlags;			// 680 kControlFlag_xxx
	UInt8								unk0681[3];						// 681
	UInt32								unk684[(0x68C - 0x684) >> 2];	// 684
	ValidBip01Names						* bipedAnims1stPerson;					// 68C
	ExtraAnim::Animation				* extraAnimation;				// 690 ExtraDataAnim::Data
	NiNode								* playerNode;					// 694 used as node if unk64A is true
	UInt32								unk698[(0x6A8-0x698) >> 2];		// 698
	TESTopic							* topic;						// 6A8
	UInt32								unk6AC[3];						// 6AC
	TESQuest							* quest;						// 6B8
	tList<BGSQuestObjective>			questObjectiveList;				// 6BC
	UInt32								unk6C4[39];				// 6C4
	TESRegion							*currentRegion;			// 760
	TESRegionList						regionsList;			// 764
	UInt32								unk770[18];				// 770
	UInt8								gameDifficulty;			// 7B8
	UInt8								pad7B9[3];				// 7B9
	bool								isHardcore;				// 7BC
	UInt8								pad7BD[3];				// 7BD
	UInt32								unk7C0[49];				// 7C0
	tList<BGSEntryPointPerkEntry>		perkEntries[74];		// 884
	UInt32								unkAD4[164];			// AD4
	CombatActors						*combatActors;			// D64
	UInt32								unkD68[3];				// D68
	UInt8								unkD74[96];				// D74
	UInt32								unkDD4[(0x0E50 - 0x0DD4) / 4];	// DD4

		// 7C4 is a byte used during Combat evaluation (Player is targetted ?), 
		// 7C6 is a boolean meaning toddler, 
		// 7C7 byte bool PCCanUsePowerArmor, Byt0E39 referenced during LoadGame
		// Used when entering FlyCam : 7E8/7EC/7F0 stores Pos, 7F0 adjusted by scaledHeight multiplied by 0698 , 7E0 stores RotZ, 7E4 RotX
		// Perks forms a list at 87C and AD4. Caravan Cards at 614 and 618. Quest Stage LogEntry at 6B0. Quest Objectives at 6BC.
		// Hardcore flag would be E38. Byte at DF0 seems to be PlayerIsInCombat
		// tList at 6C4 is cleared when there is no current quest. There is another NiNode at 069C
		// list of perk and perkRank are at 0x087C and 0x0AD4 (alt perks). 086C is cleared after equipement change.
		// D68 counts the Teammates.
		// D74: 96 bytes that are cleared when the 3D is cleared.

	bool IsThirdPerson() { return bThirdPerson ? true : false; }
	UInt32 GetMovementFlags() { return actorMover->GetMovementFlags(); }	// 11: IsSwimming, 9: IsSneaking, 8: IsRunning, 7: IsWalking, 0: keep moving
	bool IsPlayerSwimming() { return (GetMovementFlags()  >> 11) & 1; }

	static PlayerCharacter*	GetSingleton();
	bool SetSkeletonPath(const char* newPath);
	bool SetSkeletonPath_v1c(const char* newPath);	// Less worse version as used by some modders
	static void UpdateHead(void);
	QuestObjectiveTargets* GetCurrentQuestObjectiveTargets();

	void UpdateCamera(bool isCalledFromFunc21, bool _zero_skipUpdateLOD);
};

extern PlayerCharacter** g_thePlayer;

STATIC_ASSERT(offsetof(PlayerCharacter, ragDollController) == 0x0AC);
STATIC_ASSERT(offsetof(PlayerCharacter, questObjectiveList) == 0x6BC);
STATIC_ASSERT(offsetof(PlayerCharacter, bThirdPerson) == 0x64C);
STATIC_ASSERT(offsetof(PlayerCharacter, actorMover) == 0x190);
STATIC_ASSERT(sizeof(PlayerCharacter) == 0xE50);
