#include "GameObjects.h"
#include "GameRTTI.h"
#include "GameExtraData.h"
#include "GameTasks.h"
#include "GameUI.h"
#include "GameAPI.h"
#include "SafeWrite.h"
#include "NiObjects.h"

static const UInt32 s_TESObject_REFR_init	= 0x0055A2F0;	// TESObject_REFR initialization routine (first reference to s_TESObject_REFR_vtbl)
static const UInt32	s_Actor_EquipItem		= 0x0088C650;	// maybe, also, would be: 007198E0 for FOSE	4th call from the end of TESObjectREFR::RemoveItem (func5F)
static const UInt32	s_Actor_UnequipItem		= 0x0088C790;	// maybe, also, would be: 007133E0 for FOSE next sub after EquipItem
static const UInt32 s_TESObjectREFR__GetContainer	= 0x0055D310;	// First call in REFR::RemoveItem
static const UInt32 s_TESObjectREFR_Set3D	= 0x005702E0;	// void : (const char*)
static const UInt32 s_PlayerCharacter_GetCurrentQuestTargets	= 0x00952BA0;	// BuildedQuestObjectiveTargets* : (void)
static const UInt32 s_PlayerCharacter_GenerateNiNode	= 0x0094E1D0; // Func0072
static const UInt32 kPlayerUpdate3Dpatch = 0x0094EB7A;
static const UInt32 TESObjectREFR_Set3D = 0x0094EB40;
static const UInt32 ValidBip01Names_Destroy = 0x00418E00;
static const UInt32 ExtraAnimAnimation_Destroy = 0x00418D20;
static const UInt32 RefNiRefObject_ReplaceNiRefObject = 0x0066B0D0;

static const UInt32 kg_Camera1st	= 0x011E07D0;
static const UInt32 kg_Camera3rd	= 0x011E07D4;
static const UInt32 kg_Bip			= 0x011E07D8;
static const UInt8 kPlayerUpdate3DpatchFrom	= 0x0B6;
static const UInt8 kPlayerUpdate3DpatchTo	= 0x0EB;

static NiObject **	g_3rdPersonCameraNode =				(NiObject**)kg_Camera3rd;
static NiObject **	g_1stPersonCameraBipedNode =		(NiObject**)kg_Bip;
static NiObject **	g_1stPersonCameraNode =				(NiObject**)kg_Camera1st;

ExtraScript* TESObjectREFR::GetExtraScript() const
{
	BSExtraData* xData = extraDataList.GetByType(kExtraData_Script);
	if (xData)
	{
		const auto xScript = DYNAMIC_CAST(xData, BSExtraData, ExtraScript);
		if (xScript)
			return xScript;
	}

	return nullptr;
}

ScriptEventList* TESObjectREFR::GetEventList() const
{
	if (auto* extraScript = GetExtraScript())
		return extraScript->eventList;
	return nullptr;
}

PlayerCharacter** g_thePlayer = (PlayerCharacter **)0x011DEA3C;

PlayerCharacter* PlayerCharacter::GetSingleton()
{
	return *g_thePlayer;
}

QuestObjectiveTargets* PlayerCharacter::GetCurrentQuestObjectiveTargets()
{
	return (QuestObjectiveTargets *)ThisStdCall(s_PlayerCharacter_GetCurrentQuestTargets, this);
}

TESContainer* TESObjectREFR::GetContainer()
{
	if (IsActor())
		return &((TESActorBase*)baseForm)->container;
	if (baseForm->typeID == kFormType_TESObjectCONT)
		return &((TESObjectCONT*)baseForm)->container;
	return NULL;
}

bool TESObjectREFR::IsMapMarker()
{
	if (baseForm) {
		return baseForm->refID == 0x010;	// Read from the geck. Why is OBSE looking for a pointer ?
	}
	else {
		return false;
	}
}

// Less worse version as used by some modders 
bool PlayerCharacter::SetSkeletonPath_v1c(const char* newPath)
{
	if (!bThirdPerson) {
		// ###TODO: enable in first person
		return false;
	}

	return true;
}

bool TESObjectREFR::Update3D_v1c()
{
	static const UInt32 kPlayerUpdate3Dpatch = 0x0094EB7A;

	UInt8 kPlayerUpdate3DpatchFrom = 0x0B6;
	UInt8 kPlayerUpdate3DpatchTo = 0x0EB;

	if (this == *g_thePlayer) {
		// Lets try to allow unloading the player3D never the less...
		SafeWrite8(kPlayerUpdate3Dpatch, kPlayerUpdate3DpatchTo);
	}

	Set3D(NULL, true);
	ModelLoader::GetSingleton()->QueueReference(this, 1, 0);
	return true;
}

// Current basically not functioning version, but should show some progress in the end.. I hope
bool PlayerCharacter::SetSkeletonPath(const char* newPath)
{
	if (!bThirdPerson) {
		// ###TODO: enable in first person
		return false;
	}

#ifdef NVSE_CORE
#ifdef _DEBUG
	// store parent of current niNode
	/*NiNode* niParent = (NiNode*)(renderState->niNode->m_parent);

	if (renderState->niNode) renderState->niNode->Dump(0, "");

	// set niNode to NULL via BASE CLASS Set3D() method
	ThisStdCall(s_TESObjectREFR_Set3D, this, NULL, true);

	// modify model path
	if (newPath) {
		TESNPC* base = DYNAMIC_CAST(baseForm, TESForm, TESNPC);
		base->model.SetPath(newPath);
	}

	// create new NiNode, add to parent
	//*(g_bUpdatePlayerModel) = 1;

	// s_PlayerCharacter_GenerateNiNode = (MobileObject::Func0053 in Oblivion)
	NiNode* newNode = (NiNode*)ThisStdCall(s_PlayerCharacter_GenerateNiNode, this, false);	// got a body WITHOUT the head :)

	if (newNode) newNode->Dump(0, "");

	if (niParent)
		niParent->AddObject(newNode, 1);	// no complaints
	//*(g_bUpdatePlayerModel) = 0;

	newNode->SetName("Player");	// no complaints

	if (newNode) newNode->Dump(0, "");

	if (playerNode) playerNode->Dump(0, "");

	// get and store camera node
	// ### TODO: pretty this up
	UInt32 vtbl = *((UInt32*)newNode);				// ok
	UInt32 vfunc = *((UInt32*)(vtbl + 0x9C));		// ok
	NiObject* cameraNode = (NiObject*)ThisStdCall(vfunc, newNode, "Camera3rd");				// returns NULL !!!
	*g_3rdPersonCameraNode = cameraNode;

	cameraNode = (NiObject*)ThisStdCall(vfunc, (NiNode*)this->playerNode, "Camera1st");		// returns NULL !!!
	*g_1stPersonCameraNode = cameraNode;

	AnimateNiNode();*/
#endif
#endif
	return true;
}

bool TESObjectREFR::Update3D()
{
	if (this == *g_thePlayer) {
#ifdef _DEBUG
		TESModel* model = DYNAMIC_CAST(baseForm, TESForm, TESModel);
		return (*g_thePlayer)->SetSkeletonPath(model->GetModelPath());
#else
		// Lets try to allow unloading the player3D never the less...
		SafeWrite8(kPlayerUpdate3Dpatch, kPlayerUpdate3DpatchTo);
#endif
	}

	Set3D(NULL, true);
	ModelLoader::GetSingleton()->QueueReference(this, 1, false);
	return true;
}

TESObjectREFR* TESObjectREFR::Create(bool bTemp)
{
	TESObjectREFR* refr = ThisCall<TESObjectREFR*>(0x55A2F0, Game_HeapAlloc<TESObjectREFR>());
	if (bTemp) ThisCall(0x484490, refr);
	return refr;
}

TESForm* GetPermanentBaseForm(TESObjectREFR* thisObj)	// For LevelledForm, find real baseForm, not temporary one.
{
	ExtraLeveledCreature * pXCreatureData = NULL;

	if (thisObj) {
		pXCreatureData = GetByTypeCast(thisObj->extraDataList, LeveledCreature);
		if (pXCreatureData && pXCreatureData->baseForm) {
			return pXCreatureData->baseForm;
		}
	}
	if (thisObj && thisObj->baseForm) {
		return thisObj->baseForm;
	}
	return NULL;
}

__declspec(naked) float __vectorcall Point3Distance(const NiVector3& pt1, const NiVector3& pt2)
{
	__asm
	{
		movups    xmm0, [ecx]
		movups    xmm1, [edx]
		subps    xmm0, xmm1
		andps    xmm0, PS_XYZ0Mask
		mulps    xmm0, xmm0
		xorps    xmm1, xmm1
		haddps    xmm0, xmm1
		haddps    xmm0, xmm1
		comiss    xmm0, xmm1
		jz        done
		movq    xmm1, xmm0
		rsqrtss    xmm2, xmm0
		mulss    xmm1, xmm2
		mulss    xmm1, xmm2
		movss    xmm3, SS_3
		subss    xmm3, xmm1
		mulss    xmm3, xmm2
		mulss    xmm3, PS_V3_Half
		mulss    xmm0, xmm3
		done :
		retn
	}
}

// Taken from JIP LN NVSE.
__declspec(naked) float __vectorcall GetDistance3D(const TESObjectREFR* ref1, const TESObjectREFR* ref2)
{
	__asm
	{
		movups	xmm0, [ecx + 0x30]
		movups	xmm1, [edx + 0x30]
		subps	xmm0, xmm1
		mulps	xmm0, xmm0
		movhlps	xmm1, xmm0
		addss	xmm1, xmm0
		psrlq	xmm0, 0x20
		addss	xmm1, xmm0
		sqrtss	xmm0, xmm1
		retn
	}
}

__declspec(naked) void TESObjectREFR::DeleteReference()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	ecx
		push	1
		mov		eax, [ecx]
		call	dword ptr[eax + 0xC4]
		push	1
		mov		ecx, [ebp - 4]
		mov		eax, [ecx]
		call	dword ptr[eax + 0xC8]
		push	0
		push	0
		mov		ecx, [ebp - 4]
		mov		eax, [ecx]
		call	dword ptr[eax + 0x1CC]
		mov		ecx, [ebp - 4]
		cmp		byte ptr[ecx + 0xF], 0xFF
		jnz		done
		lea		eax, [ebp - 4]
		push	eax
		mov		ecx, 0x11CACB8
		CALL_EAX(0x5AE3D0)
		done:
		leave
			retn
	}
}

//Copied from JIP formerly GetNiNode(). Needed for SetMaterialProperty in Overcharge.
__declspec(naked) NiNode* TESObjectREFR::GetNiNodeJIP() const
{
	__asm
	{
		mov		eax, [ecx + 0x64]
		test	eax, eax
		jz		done
		mov		eax, [eax + 0x14]
		cmp		dword ptr[ecx + 0xC], 0x14
		jnz		done
		cmp		byte ptr[ecx + 0x64A], 0
		jnz		done
		mov		eax, [ecx + 0x694]
		done:
		retn
	}
}

/*
__declspec(naked) NiNode* __fastcall TESObjectREFR::GetNode(const char* nodeName) const
{
	__asm
	{
		//call	TESObjectREFR::GetNiNode
		test	eax, eax
		//jz		done
		//cmp[edx], 0
		//jz		done
		//mov		ecx, eax
		//call	NiNode::GetBlock
		//test	eax, eax
		//jz		done
		//xor edx, edx
		//mov		ecx, [eax]
		//cmp		dword ptr[ecx + 0xC], ADDR_ReturnThis
		//cmovnz	eax, edx
		done :
		retn
	}
}
*/
void Actor::EquipItem(TESForm * objType, UInt32 equipCount, ExtraDataList* itemExtraList, UInt32 noMessage, bool lockEquip, UInt32 playsound)
{
	ThisStdCall(s_Actor_EquipItem, this, objType, equipCount, itemExtraList, noMessage, lockEquip, playsound);
}

void Actor::UnequipItem(TESForm* objType, UInt32 unk1, ExtraDataList* itemExtraList, UInt32 noMessage, bool lockUnequip, UInt32 playsound)
{
	ThisStdCall(s_Actor_UnequipItem, this, objType, unk1, itemExtraList, noMessage, lockUnequip, playsound);
}

EquippedItemsList Actor::GetEquippedItems()
{
	EquippedItemsList itemList;
	ExtraContainerDataArray outEntryData;
	ExtraContainerExtendDataArray outExtendData;

	ExtraContainerChanges	* xChanges = static_cast <ExtraContainerChanges *>(extraDataList.GetByType(kExtraData_ContainerChanges));
	if(xChanges) {
		UInt32 count = xChanges->GetAllEquipped(outEntryData, outExtendData);
		for (UInt32 i = 0; i < count ; i++)
			itemList.push_back(outEntryData[i]->type);

	}

	return itemList;
}

ExtraContainerDataArray	Actor::GetEquippedEntryDataList()
{
	ExtraContainerDataArray itemArray;
	ExtraContainerExtendDataArray outExtendData;

	ExtraContainerChanges	* xChanges = static_cast <ExtraContainerChanges *>(extraDataList.GetByType(kExtraData_ContainerChanges));
	if(xChanges)
		xChanges->GetAllEquipped(itemArray, outExtendData);

	return itemArray;
}

ExtraContainerExtendDataArray	Actor::GetEquippedExtendDataList()
{
	ExtraContainerDataArray itemArray;
	ExtraContainerExtendDataArray outExtendData;

	ExtraContainerChanges* xChanges = static_cast<ExtraContainerChanges*>(extraDataList.GetByType(kExtraData_ContainerChanges));
	if(xChanges)
		xChanges->GetAllEquipped(itemArray, outExtendData);

	return outExtendData;
}

ContChangesEntry* Actor::GetEquippedWeaponInfo() const
{
	if (baseProcess) {
		return baseProcess->GetWeaponInfo();
	}
	return NULL;
}

TESObjectWEAP* Actor::GetEquippedWeapon() const
{
	if (baseProcess) {
		ExtraContainerChanges::EntryData* weaponInfo = baseProcess->GetWeaponInfo();
		if (weaponInfo) return (TESObjectWEAP*)weaponInfo->type;
	}
	return NULL;
}

__declspec(naked) void __fastcall SetContainerItemsHealthHook(TESContainer* container, int, float healthPerc)
{
	__asm
	{
		push	esi
		push	edi
		movss	xmm0, [esp + 0xC]
		minss	xmm0, kMaxHealth
		xorps	xmm1, xmm1
		maxss	xmm0, xmm1
		movss[esp + 0xC], xmm0
		lea		esi, [ecx + 4]
		ALIGN 16
		iterHead:
		test	esi, esi
		jz		done
		mov		edi, [esi]
		mov		esi, [esi + 4]
		test	edi, edi
		jz		iterHead
		mov		eax, [edi + 8]
		mov		ecx, [edi + 4]
		mov		dl, [ecx + 4]
		cmp		dl, kFormType_TESObjectARMO
		jz		hasHealth
		cmp		dl, kFormType_TESObjectWEAP
		jnz		clrExtra
		cmp		byte ptr[ecx + 0xF4], 0xA
		jb		hasHealth
	clrExtra :
		test	eax, eax
		jz		iterHead
		mov		dword ptr[eax + 8], 0x3F800000
		mov		edx, [eax]
		test	edx, edx
		jnz		iterHead
		mov[edi + 8], edx
		push	eax
		call	Game_HeapFree
		jmp		iterHead
		ALIGN 16
		hasHealth:
		test	eax, eax
		jnz		hasExtra
		push	0xC
		call	Game_DoHeapAlloc
		mov[edi + 8], eax
		xor edx, edx
		mov[eax], edx
		mov[eax + 4], edx
		mov		edx, [esp + 0xC]
		mov[eax + 8], edx
		jmp		iterHead
		ALIGN 16
	hasExtra:
		movss	xmm0, [esp + 0xC]
		minss	xmm0, [eax + 8]
		movss[eax + 8], xmm0
		jmp		iterHead
	done :
		pop		edi
		pop		esi
		retn	4
		ALIGN 4
		kMaxHealth :
		__asm _emit 0x77 __asm _emit 0xBE __asm _emit 0x7F __asm _emit 0x3F
	}
}

__declspec(naked) void __fastcall ShowItemMessage(TESForm* item, const char* msgStr)
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	edx
		call	TESForm::GetTheName
		cmp[eax], 0
		jz		done
		sub		esp, 0x70
		mov		edx, eax
		lea		ecx, [ebp - 0x74]
		call	StrCopy
		mov[eax], ' '
		mov		edx, [ebp - 4]
		lea		ecx, [eax + 1]
		call	StrCopy
		mov		word ptr[eax], '.'
		push	0
		push	0x40000000
		push	0
		push	0x10208E0
		push	0
		lea		eax, [ebp - 0x74]
		push	eax
		CALL_EAX(0x7052F0)
		done:
		leave
		retn
	}
}

void Actor::EquipItemAlt(ContChangesEntry* entry, UInt32 noUnequip, UInt32 noMessage, bool playsound) {

	if (!entry || !entry->type) return;

	TESForm* form = entry->type;
	SInt32 countDelta = 1;
	UInt32 typeID = form->typeID;

	ExtraDataList* xData = nullptr;

	if (entry->extendData) {

		xData = entry->extendData->m_listHead.data;
		countDelta = entry->countDelta;

	}

	this->EquipItem(form, countDelta, xData, 1, noUnequip, playsound);

	if (!noMessage) {
		const char* messageAddress = reinterpret_cast<const char*>(0x11D2A10);
		ShowItemMessage(form, messageAddress);
	}

}

const bool kInventoryType[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
};
/*
// Assuming the function prototypes based on your provided information
using InitializeContainerFn = void (*)(void* container);
using ProcessItemFn = void (*)(void* container, TESForm* form, uint32_t count, uint32_t flag);
using GetLeveledItemFn = TESForm * (*)(TESObjectREFR* refr, bool someFlag);
using UnknownFunction = int (*)(TESObjectREFR* refr);
using GetContainerChangesListFn = ExtraContainerChanges::EntryDataList* (*)(TESObjectREFR* refr);
using ProcessActor = void (*)(Actor* actor, ExtraContainerChanges::EntryData* entryData, uint32_t unknown, uint32_t noMessage);

// Pointers to functions
InitializeContainerFn InitializeContainer = reinterpret_cast<InitializeContainerFn>(0x481610);
ProcessItemFn ProcessForm = reinterpret_cast<ProcessItemFn>(0x4818E0);
GetLeveledItemFn GetLeveledItem = reinterpret_cast<GetLeveledItemFn>(0x567E10);
UnknownFunction unknown = reinterpret_cast<UnknownFunction>(0x481680);
GetContainerChangesListFn GetContainerChangesListFns = reinterpret_cast<GetContainerChangesListFn>(0x4821A0);
ProcessActor ProccessActor = reinterpret_cast<ProcessActor>(0x487F70);

void TESObjectREFR::AddItemAlt(TESForm* form, UInt32 count, float condition, UInt32 doEquip, UInt32 noMessage) {

	char localContainer[16]; // Assuming you need a local buffer similar to the `sub esp, 0x10`
	InitializeContainer(localContainer);

	UInt8 typeID = form->typeID;
	if (typeID == kFormType_TESLevItem) {

		TESForm* item = GetLeveledItem(this, false);
		ProccessActor(static_cast<Actor*>(this), reinterpret_cast<ExtraContainerChanges::EntryData*>(item), count, noMessage);

	}
	else if (typeID == kFormType_BGSListForm) {

		BGSListForm* bgsListForm = static_cast<BGSListForm*>(form);

		for (auto it = bgsListForm->list.begin(); it != bgsListForm->list.end(); ++it) {

			TESForm* currentItem = *it;
			if (kInventoryType[currentItem->typeID]) {
				ProcessForm(localContainer, currentItem, count, 0);
			}

		}

	}
	else {

		if (!kInventoryType[typeID]) {
			return;
		}
		ProcessForm(localContainer, form, count, 0);

	}

	SetContainerItemsHealthHook((TESContainer*)form, 0, condition);
	auto entryList = GetContainerChangesListFns(this);

	if (doEquip) {

		if (entryList) {
			
			for (auto iter = entryList->Begin(); !iter.End(); ++iter) {

				auto entry = iter.Get();

				auto entryData = entryList->FindForItem(entry->type);
				if (entryData) {
					static_cast<Actor*>(this)->EquipItemAlt(entryData, 0, noMessage);
				}
			}
		}
	}

}
*/
__declspec(naked) void TESObjectREFR::AddItemAlt(TESForm* form, UInt32 count, float condition, UInt32 doEquip, UInt32 noMessage)
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	ecx
		sub		esp, 0x10
		push	esi
		lea		ecx, [ebp - 0x10]
		CALL_EAX(0x481610)
		mov		ecx, [ebp + 8]
		movzx	eax, byte ptr[ecx + 4]
		cmp		al, kFormType_TESLevItem
		jz		lvlItem
		cmp		al, kFormType_BGSListForm
		jz		itemList
		cmp		kInventoryType[eax], 0
		jz		done
		push	0
		push	dword ptr[ebp + 0xC]
		push	ecx
		lea		ecx, [ebp - 0x10]
		CALL_EAX(0x4818E0)
		jmp		doMove
		lvlItem :
		push	0
			mov		ecx, [ebp - 4]
			CALL_EAX(0x567E10)
			push	0
			lea		ecx, [ebp - 0x10]
			push	ecx
			push	dword ptr[ebp + 0xC]
			push	eax
			mov		ecx, [ebp + 8]
			add		ecx, 0x30
			CALL_EAX(0x487F70)
			jmp		doMove
			itemList :
		mov		esi, [ebp + 8]
			add		esi, 0x18
			ALIGN 16
			listIter :
			test	esi, esi
			jz		doMove
			mov		ecx, [esi]
			mov		esi, [esi + 4]
			test	ecx, ecx
			jz		listIter
			movzx	eax, byte ptr[ecx + 4]
			cmp		kInventoryType[eax], 0
			jz		listIter
			push	0
			push	dword ptr[ebp + 0xC]
			push	ecx
			lea		ecx, [ebp - 0x10]
			CALL_EAX(0x4818E0)
			jmp		listIter
			ALIGN 16
			doMove:
		push	dword ptr[ebp + 0x10]
			lea		ecx, [ebp - 0x10]
			call	SetContainerItemsHealthHook
			push	dword ptr[ebp + 0x18]
			push	dword ptr[ebp - 4]
			lea		ecx, [ebp - 0x10]
			CALL_EAX(0x4821A0)
			cmp		byte ptr[ebp + 0x14], 0
			jz		done
			mov		ecx, [ebp - 4]
			mov		eax, [ecx]
			cmp		dword ptr[eax + 0x100], 0x8D0360
			jnz		done
			call	TESObjectREFR::GetContainerChangesList
			test	eax, eax
			jz		done
			mov[ebp - 0x14], eax
			lea		esi, [ebp - 0xC]
			ALIGN 16
			eqpIter:
		test	esi, esi
			jz		done
			mov		eax, [esi]
			mov		esi, [esi + 4]
			test	eax, eax
			jz		eqpIter
			mov		edx, [eax + 4]
			mov		ecx, [ebp - 0x14]
			call	ContChangesEntryList::EntryDataList::FindForItem
			test	eax, eax
			jz		eqpIter
			push	dword ptr[ebp + 0x18]
			push	0
			push	eax
			mov		ecx, [ebp - 4]
			call	Actor::EquipItemAlt
			jmp		eqpIter
			ALIGN 16
			done:
		lea		ecx, [ebp - 0x10]
			CALL_EAX(0x481680)
			pop		esi
			leave
			retn	0x14
	}
}

__declspec(naked) SInt32 __fastcall GetFormCount(TESContainer::FormCountList* formCountList, ContChangesEntryList* objList, TESForm* form)
{
	__asm
	{
		push	esi
		push	edi
		mov		esi, [esp + 0xC]
		xor edi, edi
		ALIGN 16
		contIter:
		test	ecx, ecx
			jz		xtraIter
			mov		eax, [ecx]
			mov		ecx, [ecx + 4]
			test	eax, eax
			jz		contIter
			cmp[eax + 4], esi
			jnz		contIter
			add		edi, [eax]
			jmp		contIter
			ALIGN 16
			xtraIter:
		test	edx, edx
			jz		done
			mov		ecx, [edx]
			mov		edx, [edx + 4]
			test	ecx, ecx
			jz		xtraIter
			cmp[ecx + 8], esi
			jnz		xtraIter
			mov		esi, ecx
			test	edi, edi
			jz		noCont
			call	ContChangesEntry::HasExtraLeveledItem
			test	al, al
			jnz		noCont
			add		edi, [esi + 4]
			js		retnZero
			jmp		done
			ALIGN 16
			noCont:
		mov		edi, [esi + 4]
			test	edi, edi
			jge		done
			retnZero :
		xor edi, edi
			done :
		mov		eax, edi
			pop		edi
			pop		esi
			retn	4
	}
}

__declspec(naked) SInt32 TESObjectREFR::GetItemCount(TESForm* form) const
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	ecx
		mov		ecx, [ecx + 0x20]
		call	TESForm::GetContainer
		test	eax, eax
		jz		done
		pop		ecx
		add		eax, 4
		push	eax
		call	TESObjectREFR::GetContainerChangesList
		mov		edx, eax
		mov		eax, [ebp + 8]
		test	eax, eax
		jz		done
		cmp		byte ptr[eax + 4], kFormType_BGSListForm
		jz		itemList
		push	eax
		mov		ecx, [ebp - 4]
		call	GetFormCount
		done :
		leave
			retn	4
			__asm _emit 0x66 __asm _emit 0x90
		itemList :
			push	edx
			push	esi
			push	edi
			lea		esi, [eax + 0x18]
			xor edi, edi
			iterHead :
		test	esi, esi
			jz		iterEnd
			mov		eax, [esi]
			mov		esi, [esi + 4]
			test	eax, eax
			jz		iterHead
			push	eax
			mov		edx, [ebp - 8]
			mov		ecx, [ebp - 4]
			call	GetFormCount
			add		edi, eax
			jmp		iterHead
			ALIGN 16
			iterEnd:
		mov		eax, edi
			pop		edi
			pop		esi
			leave
			retn	4
	}
}

__declspec(naked) ContChangesEntryList* TESObjectREFR::GetContainerChangesList() const
{
	__asm
	{
		push	kXData_ExtraContainerChanges
		add		ecx, 0x44
		call	BaseExtraList::GetByType
		test	eax, eax
		jz		done
		mov		eax, [eax + 0xC]
		test	eax, eax
		jz		done
		mov		eax, [eax]
		done:
		retn
	}
}
__declspec(naked) ContChangesEntry* TESObjectREFR::GetContainerChangesEntry(TESForm * itemForm) const
{
	__asm
	{
		push	kXData_ExtraContainerChanges
		add		ecx, 0x44
		call	BaseExtraList::GetByType
		test	eax, eax
		jz		done
		mov		eax, [eax + 0xC]
		test	eax, eax
		jz		done
		mov		ecx, [eax]
		mov		edx, [esp + 4]
		ALIGN 16
		itemIter:
		test	ecx, ecx
		jz		retnNULL
		mov		eax, [ecx]
		mov		ecx, [ecx + 4]
		test	eax, eax
		jz		itemIter
		cmp[eax + 8], edx
		jnz		itemIter
		retn	4
		retnNULL:
		xor eax, eax
		done :
		retn	4
	}
}

bool TESObjectREFR::GetInventoryItems(InventoryItemsMap &invItems)
{
	TESContainer *container = GetContainer();
	if (!container) return false;
	ExtraContainerChanges *xChanges = (ExtraContainerChanges*)extraDataList.GetByType(kExtraData_ContainerChanges);
	ExtraContainerChanges::EntryDataList *entryList = (xChanges && xChanges->data) ? xChanges->data->objList : NULL;
	if (!entryList) return false;

	TESForm *item;
	SInt32 contCount, countDelta;
	ExtraContainerChanges::EntryData *entry;

	for (auto contIter = container->formCountList.Begin(); !contIter.End(); ++contIter)
	{
		item = contIter->form;
		if ((item->typeID == kFormType_TESLevItem) || invItems.HasKey(item))
			continue;
		contCount = container->GetCountForForm(item);
		if (entry = entryList->FindForItem(item))
		{
			countDelta = entry->countDelta;
			if (entry->HasExtraLeveledItem())
				contCount = countDelta;
			else contCount += countDelta;
		}
		if (contCount > 0)
			invItems.Emplace(item, contCount, entry);
	}

	for (auto xtraIter = entryList->Begin(); !xtraIter.End(); ++xtraIter)
	{
		entry = xtraIter.Get();
		item = entry->type;
		if (invItems.HasKey(item))
			continue;
		countDelta = entry->countDelta;
		if (countDelta > 0)
			invItems.Emplace(item, countDelta, entry);
	}

	return !invItems.Empty();
}

ExtraDroppedItemList* TESObjectREFR::GetDroppedItems()
{
	return static_cast<ExtraDroppedItemList*>(extraDataList.GetByType(kExtraData_DroppedItemList));
}

// Code by JIP
double TESObjectREFR::GetHeadingAngle(const TESObjectREFR* to) const
{
	auto const from = this;
	double result = (atan2(to->posX - from->posX, to->posY - from->posY) - from->rotZ) * 57.29577951308232;
	if (result < -180)
		result += 360;
	else if (result > 180)
		result -= 360;
	return result;
}

__declspec(naked) NiAVObject* __fastcall TESObjectREFR::GetNifBlock(TESObjectREFR* thisObj, UInt32 pcNode, const char* blockName)
{
	__asm
	{
		test	dl, dl
		jz		notPlayer
		cmp		dword ptr[ecx + 0xC], 0x14
		jnz		notPlayer
		test	dl, 1
		jz		get1stP
		mov		eax, [ecx + 0x64]
		test	eax, eax
		jz		done
		mov		eax, [eax + 0x14]
		jmp		gotRoot
		get1stP :
		mov		eax, [ecx + 0x694]
		jmp		gotRoot
		notPlayer :
		mov		eax, [ecx]
		call	dword ptr[eax + 0x1D0]
		gotRoot :
		test	eax, eax
		jz		done
		mov		edx, [esp + 4]
		cmp[edx], 0
		jz		done
		mov		ecx, eax
		call	NiNode::GetBlock
		done :
		retn	4
	}
}

__declspec(naked) bool __fastcall TESObjectREFR::GetInSameCellOrWorld(TESObjectREFR* target) const
{
	__asm
	{
		mov		eax, [ecx + 0x40]
		test	eax, eax
		jnz		hasCell1
		push	edx
		push	kExtraData_PersistentCell
		add		ecx, 0x44
		call	BaseExtraList::GetByType
		pop		edx
		test	eax, eax
		jz		done
		mov		eax, [eax + 0xC]
	hasCell1:
		mov		ecx, [edx + 0x40]
		test	ecx, ecx
		jnz		hasCell2
		push	eax
		push	kExtraData_PersistentCell
		lea		ecx, [edx + 0x44]
		call	BaseExtraList::GetByType
		pop		edx
		test	eax, eax
		jz		done
		mov		ecx, [eax + 0xC]
		mov		eax, edx
	hasCell2 :
		cmp		eax, ecx
		jz		retnTrue
		mov		eax, [eax + 0xC0]
		test	eax, eax
		jz		done
		cmp		eax, [ecx + 0xC0]
	retnTrue:
		setz	al
	done :
		retn
	}
}

// Code by JIP
__declspec(naked) float __vectorcall TESObjectREFR::GetDistance(TESObjectREFR* target) const
{
	__asm
	{
		push	ecx
		push	edx
		call	TESObjectREFR::GetInSameCellOrWorld
		pop		edx
		pop		ecx
		test	al, al
		jz		fltMax
		add		ecx, 0x30
		add		edx, 0x30
		jmp		Point3Distance
	fltMax :
		mov		eax, 0x7F7FFFFF
		movd	xmm0, eax
		retn
	}
}

__declspec(naked) NiNode* __fastcall TESObjectREFR::GetNode(const char* nodeName) const
{
	__asm
	{
		mov		eax, [ecx + 0x64]
		test	eax, eax
		jz		done
		mov		eax, [eax + 0x14]
		test	eax, eax
		jz		done
		cmp[edx], 0
		jz		done
		mov		ecx, eax
		call	NiNode::GetBlock
		test	eax, eax
		jz		done
		xor edx, edx
		mov		ecx, [eax]
		cmp		dword ptr[ecx + 0xC], ADDR_ReturnThis
		cmovnz	eax, edx
		done :
		retn
	}
}

void Actor::SetWantsWeaponOut(bool wantsWeaponOut)
{
	ThisStdCall(0x8A6840, this, (UInt8)wantsWeaponOut);
}

void PlayerCharacter::UpdateCamera(bool isCalledFromFunc21, bool _zero_skipUpdateLOD)
{
	ThisStdCall(0x94AE40, this, (UInt8)isCalledFromFunc21, (UInt8)_zero_skipUpdateLOD);
}

__declspec(naked) void Actor::RefreshAnimData()
{
	__asm
	{
		push	esi
		mov		esi, [ecx + 0x68]
		test	esi, esi
		jz		done
		cmp		dword ptr[esi + 0x28], 0
		jnz		done
		mov		eax, [esi + 0x1C0]
		test	eax, eax
		jz		done
		push	ecx
		cmp		byte ptr[esi + 0x135], 0
		jz		skipBlend1
		cmp		dword ptr[eax + 0xF0], 0
		jz		skipBlend1
		push	0
		push	4
		mov		ecx, eax
		CALL_EAX(0x4994F0)
		mov		eax, [esi + 0x1C0]
		skipBlend1:
		push	0
		mov		ecx, eax
		CALL_EAX(0x499240)
		pop		ecx
		cmp		dword ptr[ecx + 0xC], 0x14
		jnz		done
		cmp		byte ptr[esi + 0x135], 0
		mov		esi, [ecx + 0x690]
		jz		skipBlend2
		cmp		dword ptr[esi + 0xF0], 0
		jz		skipBlend2
		push	0
		push	4
		mov		ecx, esi
		CALL_EAX(0x4994F0)
		skipBlend2:
		push	0
		mov		ecx, esi
		CALL_EAX(0x499240)
		done :
		pop		esi
		retn
	}
}