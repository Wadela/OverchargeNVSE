#include "NiNodes.h"

#include "GameScript.h"
#include "MemoizedMap.h"

//std::span<TESAnimGroup::AnimGroupInfo> g_animGroups = { reinterpret_cast<TESAnimGroup::AnimGroupInfo*>(0x011977D8), AnimGroupID::kAnimGroup_Max };
std::span<TESAnimGroup::AnimGroupInfo> g_animGroupInfos = { reinterpret_cast<TESAnimGroup::AnimGroupInfo*>(0x11977D8), 245 };

#if RUNTIME
void TextureFormat::InitFromD3DFMT(UInt32 fmt)
{
	typedef void (* _D3DFMTToTextureFormat)(UInt32 d3dfmt, TextureFormat * dst);
	_D3DFMTToTextureFormat D3DFMTToTextureFormat = (_D3DFMTToTextureFormat)0x00E7C1E0;
	
	D3DFMTToTextureFormat(fmt, this);
}

static const UInt32 kNiObjectNET_SetNameAddr = 0x00A5B690;

void NiObjectNET::SetName(const char* newName)
{
	// uses cdecl, not stdcall
	_asm
	{
		push newName
		// mov ecx, this (already)
		call kNiObjectNET_SetNameAddr
		add esp, 4
	}
	// OBSE : ThisStdCall(kNiObjectNET_SetNameAddr, this, newName);
}

const char* TESAnimGroup::StringForAnimGroupCode(UInt32 groupCode)
{
	return (groupCode < AnimGroupID::kAnimGroup_Max) ? g_animGroupInfos[groupCode].name : NULL;
}

MemoizedMap<const char*, UInt32> s_animGroupMap;

UInt32 TESAnimGroup::AnimGroupForString(const char* groupName)
{
	return s_animGroupMap.Get(groupName, [](const char* groupName)
	{
		for (UInt32 i = 0; i < kAnimGroup_Max; i++) 
		{
			if (!_stricmp(g_animGroupInfos[i].name, groupName))
			{
				return i;
			}
		}
		return static_cast<UInt32>(kAnimGroup_Max);
	});
}

void DumpAnimGroups(void)
{
	for (UInt32 i = 0; i < AnimGroupID::kAnimGroup_Max; i++) {
		_MESSAGE("%d,%s", i , g_animGroupInfos[i].name);
		//if (!_stricmp(s_animGroupInfos[i].name, "JumpLandRight"))
		//	break;
	}
}

//Copied from JIP LN NVSE (Lines 65 - 136) - Needed for Overcharge NVSE
const NiUpdateData kNiUpdateData;

__declspec(naked) void NiAVObject::UpdateJIP() 
{
	__asm
	{
		push	ecx
		push	0
		push	offset kNiUpdateData
		mov		eax, [ecx]
		call	dword ptr[eax + 0xA4]
		pop		ecx
		mov		ecx, [ecx + 0x18]
		test	ecx, ecx
		jz		done
		mov		eax, [ecx]
		call	dword ptr[eax + 0xFC]
		done:
		retn
	}
}

__declspec(naked) void __fastcall NiGeometry::AddProperty(NiProperty* niProperty)
{
	__asm
	{
		lock inc dword ptr[edx + 4]
		push	ecx
		push	edx
		CALL_EAX(0x43A010)
		pop		dword ptr[eax + 8]
		pop		ecx
		inc		dword ptr[ecx + 0x2C]
		mov		edx, [ecx + 0x24]
		mov[ecx + 0x24], eax
		test	edx, edx
		jz		emptyList
		mov[eax], edx
		mov[edx + 4], eax
		retn
		emptyList :
		mov[ecx + 0x28], eax
			retn
	}
}

#endif

__declspec(noinline) void __vectorcall NiMaterialProperty::SetTraitValue(UInt32 traitID, float value)
{
	switch (traitID)
	{
	case 0:
	case 1:
	case 2:
		specularRGB[traitID] = value;
		break;
	case 3:
	case 4:
	case 5:
		emissiveRGB[traitID - 3] = value;
		break;
	case 6:
		glossiness = value;
		break;
	case 7:
		alpha = value;
		break;
	default:
		emitMult = value;
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

//Copied from JIP LN NVSE - Needed for Overcharge NVSE
NiAlphaProperty* s_alphaProperty;