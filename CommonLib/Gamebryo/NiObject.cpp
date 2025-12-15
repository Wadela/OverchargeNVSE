#include "NiObject.hpp"

#define CALL_EAX(addr) __asm mov eax, addr __asm call eax

enum  //Memory Addresses
{
	kAddr_AddExtraData = 0x40FF60,
	kAddr_RemoveExtraType = 0x410140,
	kAddr_LoadModel = 0x447080,
	kAddr_ApplyAmmoEffects = 0x59A030,
	kAddr_MoveToMarker = 0x5CCB20,
	kAddr_ApplyPerkModifiers = 0x5E58F0,
	kAddr_ReturnThis = 0x6815C0,
	kAddr_PurgeTerminalModel = 0x7FFE00,
	kAddr_EquipItem = 0x88C650,
	kAddr_UnequipItem = 0x88C790,
	kAddr_ReturnTrue = 0x8D0360,
	kAddr_TileGetFloat = 0xA011B0,
	kAddr_TileSetFloat = 0xA012D0,
	kAddr_TileSetString = 0xA01350,
	kAddr_InitFontInfo = 0xA12020,
};

// 0x6532C0
bool NiObject::IsKindOf(const NiRTTI& apRTTI) const {
	return GetRTTI()->IsKindOf(apRTTI);
}

// 0x6532C0
bool NiObject::IsKindOf(const NiRTTI* const apRTTI) const {
	return GetRTTI()->IsKindOf(apRTTI);
}

// 0x45BAF0
bool NiObject::IsExactKindOf(const NiRTTI* const apRTTI) const {
	return GetRTTI()->IsExactKindOf(apRTTI);
}

// 0x45BAF0
bool NiObject::IsExactKindOf(const NiRTTI& apRTTI) const {
	return GetRTTI()->IsExactKindOf(apRTTI);
}

// 0x45BAD0
bool NiObject::IsExactKindOf(const NiRTTI& apRTTI, NiObject* apObject) {
	return apObject && apObject->IsExactKindOf(apRTTI);
}

// 0x45BAD0
bool NiObject::IsExactKindOf(const NiRTTI* const apRTTI, NiObject* apObject) {
	return apObject && apObject->IsExactKindOf(apRTTI);
}

__declspec(naked) NiObject* __fastcall NiObject::HasBaseType(const NiRTTI* baseType)
{
	__asm
	{
		mov		eax, [ecx]
		call	dword ptr[eax + 8]
		ALIGN 16
		iterHead:
		test	eax, eax
			jz		done
			cmp		eax, edx
			mov		eax, [eax + 4]
			jnz		iterHead
			mov		eax, ecx
			done :
		retn
	}
}