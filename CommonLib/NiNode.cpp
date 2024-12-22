#include "NiNode.hpp"
#include "NiCullingProcess.hpp"

// Added By Wadel - Full Credit to JIP LN NVSE

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


NiNode* NiNode::Create(UInt16 ausChildCount) {
	return NiCreate<NiNode, 0xA5ECB0>(ausChildCount);
}

UInt32 NiNode::GetArrayCount() const {
	return m_kChildren.GetSize();
}

UInt32 NiNode::GetChildCount() const {
	return m_kChildren.GetEffectiveSize();
}

NiAVObject* NiNode::GetAt(UInt32 i) const {
	return m_kChildren.GetAt(i).m_pObject;
}

NiAVObject* NiNode::GetAtSafely(UInt32 i) const {
	if (GetArrayCount() <= i)
		return nullptr;

	return GetAt(i);
}

NiAVObject* NiNode::GetLastChild() {
	if (GetChildCount() == 0)
		return nullptr;

	return GetAt(GetChildCount() - 1);
}

// 0x4ADD70
void NiNode::CompactChildArray() {
	m_kChildren.Compact();
	m_kChildren.UpdateSize();
}

NiNode* NiNode::FindNodeByName(const NiFixedString& akName) {
	if (m_kName.m_kHandle && m_kName == akName) {
		return this;
	}

	if (!GetArrayCount()) {
		return nullptr;
	}

	for (UInt32 i = 0; i < GetArrayCount(); i++) {
		NiAVObject* pkChild = GetAt(i);
		if (pkChild && IS_NODE(pkChild)) {
			if (NiNode* pkObject = pkChild->NiDynamicCast<NiNode>()->FindNodeByName(akName)) {
				return pkObject;
			}
		}
	}

	return nullptr;
}

// 0xA5E3A0
void NiNode::UpdatePropertiesUpward(NiPropertyState*& apParentState) {
	NiPropertyState* pNewState = nullptr;

	SetHasPropertyController(HasPropertyController());

	if (GetParent())
		GetParent()->UpdatePropertiesUpward(pNewState);
	else
		pNewState = new NiPropertyState();

	PushLocalProperties(pNewState, false, apParentState);

	if (pNewState != apParentState)
		delete pNewState;
}

// 0xA5DD70
void NiNode::UpdateDownwardPassEx(NiUpdateData& arData, UInt32 auiFlags) {
	ThisStdCall(0xA5DD70, this, &arData, auiFlags);
}

// 0xA5DED0
void NiNode::UpdateSelectedDownwardPassEx(NiUpdateData& arData, UInt32 auiFlags) {
	ThisStdCall(0xA5DED0, this, &arData, auiFlags);
}

// 0xA5E1F0
void NiNode::UpdateRigidDownwardPassEx(NiUpdateData& arData, UInt32 auiFlags) {
	bool bUpdateControllers = arData.bUpdateControllers;
	arData.bUpdateControllers = GetSelUpdController();
	UpdateObjectControllers(arData, 1);
	arData.bUpdateControllers = bUpdateControllers;

	if (GetSelUpdTransforms())
		UpdateWorldData(arData);

	if (GetAppCulled())
		auiFlags |= 1u;

	for (UInt16 i = 0; i < GetChildCount(); ++i) {
		NiAVObject* pChild = this->m_kChildren.GetAt(i);
		if (pChild && pChild->GetSelectiveUpdate())
			pChild->UpdateRigidDownwardPass(arData, auiFlags);
	}
}

// 0xA5D940
void NiNode::ApplyTransformEx(NiMatrix3& kMat, NiPoint3& kTrn, bool abOnLeft) {
	ThisStdCall(0xA5D940, this, &kMat, &kTrn, abOnLeft);
}

void NiNode::SetFlagRecurse(NiNode* apNode, UInt32 auiFlag, bool abSet) {
	apNode->SetBit(auiFlag, abSet);

	for (UInt32 i = 0; i < apNode->GetArrayCount(); i++) {
		NiAVObject* pChild = apNode->GetAt(i);
		if (pChild) {
			pChild->SetBit(auiFlag, abSet);

			if (IS_NODE(pChild)) {
				SetFlagRecurse(static_cast<NiNode*>(pChild), auiFlag, abSet);
			}
		}
	}
}

__declspec(naked) NiAVObject* __fastcall NiNode::GetBlockByName(const char* nameStr)	//	str of NiString
{
	__asm
	{
		movzx	eax, word ptr[ecx + 0xA6]
		test	eax, eax
		jz		done
		push	esi
		push	edi
		mov		esi, [ecx + 0xA0]
		mov		edi, eax
		ALIGN 16
		iterHead:
		dec		edi
			js		iterEnd
			mov		eax, [esi]
			add		esi, 4
			test	eax, eax
			jz		iterHead
			cmp[eax + 8], edx
			jz		found
			mov		ecx, [eax]
			cmp		dword ptr[ecx + 0xC], kAddr_ReturnThis
			jnz		iterHead
			mov		ecx, eax
			call	NiNode::GetBlockByName
			test	eax, eax
			jz		iterHead
			found :
		pop		edi
			pop		esi
			retn
			ALIGN 16
			iterEnd :
			xor eax, eax
			pop		edi
			pop		esi
			done :
		retn
	}
}

__declspec(naked) NiAVObject* __fastcall NiNode::GetBlock(const char* blockName)
{
	__asm
	{
		cmp[edx], 0
		jz		retnNULL
		push	ecx
		push	edx
		CALL_EAX(0xA5B690)
		pop		ecx
		pop		ecx
		test	eax, eax
		jz		done
		lock dec dword ptr[eax - 8]
		jz		retnNULL
		cmp[ecx + 8], eax
		jz		found
		mov		edx, eax
		call	NiNode::GetBlockByName
		retn
		found :
		mov		eax, ecx
			retn
			retnNULL :
		xor eax, eax
			done :
		retn
	}
}

__declspec(naked) NiNode* __fastcall NiNode::GetNode(const char* nodeName)
{
	__asm
	{
		call	NiNode::GetBlock
		test	eax, eax
		jz		done
		xor edx, edx
		mov		ecx, [eax]
		cmp		dword ptr[ecx + 0xC], kAddr_ReturnThis
		cmovnz	eax, edx
		done :
		retn
	}
}