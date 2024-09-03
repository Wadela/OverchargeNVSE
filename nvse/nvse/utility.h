#pragma once

#include <intrin.h>
#include "Utilities.h" // for ThisStdCall
#include <bit>

#define CALL_EAX(addr) __asm mov eax, addr __asm call eax

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret ThisCall(UInt32 _addr, void* _this, Args ...args)
{
	return ((T_Ret(__thiscall*)(void*, Args...))_addr)(_this, std::forward<Args>(args)...);
}

UInt32 GetFirstFormIDForModIndex(UInt32 modIndex);

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

const char* __cdecl GetNiFixedString(const char* inStr);

class NiFixedString
{
	const char* str;

	UInt32* Meta() const { return (UInt32*)(str - 8); }
	/*
	void Set(const char* inStr)
	{
		str = inStr;
		if (str) InterlockedIncrement(Meta());
	}
	*/
	void Unset()
	{
		if (str)
		{
			InterlockedDecrement(Meta());
			str = nullptr;
		}
	}

public:
	NiFixedString() : str(nullptr) {}

	explicit NiFixedString(const char* data): str(data){}

	NiFixedString(const NiFixedString& inStr) { Set(inStr.str); }
	~NiFixedString() { Unset(); }


	void Set(const char* newString){ThisStdCall(0xA2E750, this, newString);}

	const char* CStr() const
	{
		return str;
	}

	const char* Get() const { return str ? str : "NULL"; }

	UInt32 Length() const { return str ? Meta()[1] : 0; }

	explicit operator bool() const { return str != nullptr; }

	operator const char* () const { return str; }

	const char* operator*() const { return str; }

	inline void operator=(const char* inStr)
	{
		Unset();
		str = GetNiFixedString(inStr);
	}
	inline void operator=(const NiFixedString& inStr)
	{
		if (str != inStr.str)
			Set(inStr.str);
	}

	inline bool operator==(const NiFixedString& rhs) const { return str == rhs.str; }
	inline bool operator<(const NiFixedString& rhs) const { return str < rhs.str; }

	UInt32 RefCount() const { return str ? Meta()[0] : 0; }
};

typedef void* (*memcpy_t)(void*, const void*, size_t);
extern memcpy_t _memcpy, _memmove;

//	Workaround for bypassing the compiler calling the d'tor on function-scope objects.
template <typename T> class TempObject
{
	alignas(T) UInt8	objData[sizeof(T)];

public:
	TempObject() { Reset(); }
	TempObject(const T& src) { memcpy((void*)this, (const void*)&src, sizeof(T)); }

	template <typename ...Args>
	TempObject(Args&& ...args)
	{
		new (this) T(std::forward<Args>(args)...);
	}

	void Reset() { new (this) T(); }

	void Destroy() { (*this)().~T(); }

	T& operator()() { return *(reinterpret_cast<T*>(this)); }
	T* operator*() { return reinterpret_cast<T*>(this); }
	T* operator->() { return reinterpret_cast<T*>(this); }

	inline operator T& () { return *(reinterpret_cast<T*>(this)); }

	TempObject& operator=(const T& rhs)
	{
		memcpy((void*)this, (const void*)&rhs, sizeof(T));
		return *this;
	}
	TempObject& operator=(const TempObject& rhs)
	{
		memcpy((void*)this, (const void*)&rhs, sizeof(T));
		return *this;
	}
};

//	Assign rhs to lhs, bypassing operator=
template <typename T> __forceinline void RawAssign(const T &lhs, const T &rhs)
{
	struct Helper
	{
		UInt8	bytes[sizeof(T)];
	};
	*(Helper*)&lhs = *(Helper*)&rhs;
}

//	Swap lhs and rhs, bypassing operator=
template <typename T> __forceinline void RawSwap(const T &lhs, const T &rhs)
{
	struct Helper
	{
		UInt8	bytes[sizeof(T)];
	}
	temp = *(Helper*)&lhs;
	*(Helper*)&lhs = *(Helper*)&rhs;
	*(Helper*)&rhs = temp;
}

// These are used for 10h aligning segments in ASM code (massive performance gain, particularly with loops).
#define EMIT(bt) __asm _emit bt
#define NOP_0x1 EMIT(0x90)
#define NOP_0x2 EMIT(0x66) EMIT(0x90)
#define NOP_0x3 EMIT(0x0F) EMIT(0x1F) EMIT(0x00)
#define NOP_0x4 EMIT(0x0F) EMIT(0x1F) EMIT(0x40) EMIT(0x00)
#define NOP_0x5 EMIT(0x0F) EMIT(0x1F) EMIT(0x44) EMIT(0x00) EMIT(0x00)
#define NOP_0x6 EMIT(0x66) EMIT(0x0F) EMIT(0x1F) EMIT(0x44) EMIT(0x00) EMIT(0x00)
#define NOP_0x7 EMIT(0x0F) EMIT(0x1F) EMIT(0x80) EMIT(0x00) EMIT(0x00) EMIT(0x00) EMIT(0x00)
#define NOP_0x8 EMIT(0x0F) EMIT(0x1F) EMIT(0x84) EMIT(0x00) EMIT(0x00) EMIT(0x00) EMIT(0x00) EMIT(0x00)
#define NOP_0x9 EMIT(0x66) EMIT(0x0F) EMIT(0x1F) EMIT(0x84) EMIT(0x00) EMIT(0x00) EMIT(0x00) EMIT(0x00) EMIT(0x00)
#define NOP_0xA NOP_0x5 NOP_0x5
#define NOP_0xB NOP_0x5 NOP_0x6
#define NOP_0xC NOP_0x6 NOP_0x6
#define NOP_0xD NOP_0x6 NOP_0x7
#define NOP_0xE NOP_0x7 NOP_0x7
#define NOP_0xF NOP_0x7 NOP_0x8

#define GET_N_BYTE(a, n) ((a >> (n * 8)) & 0xFF)

#define EMIT_W(a) EMIT(GET_N_BYTE(a, 0)) EMIT(GET_N_BYTE(a, 1))
#define EMIT_DW(a) EMIT(GET_N_BYTE(a, 0)) EMIT(GET_N_BYTE(a, 1)) EMIT(GET_N_BYTE(a, 2)) EMIT(GET_N_BYTE(a, 3))

alignas(16) const UInt32 kASinConsts[] =
{
	0xBC996E30, 0x3D981627, 0x3E593484, 0x3FC90FDB, 0x34000000, 0x3F800000, 0x40490FDB, 0x03800000,
	0x37202A00, 0x37E24000, 0x38333600, 0x386E4C00, 0x38913200, 0x38A84000, 0x38BC5200, 0x38CDAC00,
	0x38DC5000, 0x38E8C000, 0x38F2C800, 0x38FAA800, 0x39005600, 0x39029000, 0x3903C800, 0x39046400,
	0x39042C00, 0x39036800, 0x39022800, 0x39003400, 0x38FB7000, 0x38F60800, 0x38EFD800, 0x38E8B800,
	0x38E15800, 0x38D96000, 0x38D0C000, 0x38C83000, 0x38BF2000, 0x38B59800, 0x38AC5800, 0x38A2E000,
	0x38998000, 0x388FD000, 0x38866000, 0x387AA000, 0x38678000, 0x3855E000, 0x3844A000, 0x38338000,
	0x38234000, 0x3812E000, 0x38042000, 0x37EB4000, 0x37CF4000, 0x37B64000, 0x379D4000, 0x37874000,
	0x37658000, 0x373F8000, 0x371A8000, 0x36F90000, 0x36C30000, 0x36900000, 0x364E0000, 0x360E0000,
	0x35B40000, 0x35500000, 0x34D00000, 0x33C00000, 0x34000000, 0x34800000, 0x35400000, 0x35A80000,
	0x36040000, 0x36400000, 0x367C0000, 0x36A60000, 0x36D60000, 0x37040000, 0x371D0000, 0x37390000,
	0x37560000, 0x37750000, 0x378B0000, 0x379C0000, 0x37AC8000, 0x37BE8000, 0x37D08000, 0x37E30000,
	0x37F58000, 0x3803C000, 0x380D0000, 0x3815C000, 0x381EC000, 0x38274000, 0x382F8000, 0x38380000,
	0x383F8000, 0x38470000, 0x384E8000, 0x3854C000, 0x385B0000, 0x38604000, 0x38658000, 0x3869C000,
	0x386D4000, 0x38708000, 0x3872C000, 0x38744000, 0x38750000, 0x38750000, 0x38740000, 0x38724000,
	0x38700000, 0x386C0000, 0x3867C000, 0x3862C000, 0x385C8000, 0x38558000, 0x384D8000, 0x38450000,
	0x383B0000, 0x38318000, 0x38260000, 0x381A0000, 0x380D8000, 0x38010000, 0x37E70000, 0x37CB0000,
	0x37AF0000, 0x37930000, 0x376C0000, 0x37340000, 0x37020000, 0x36A00000, 0x36180000, 0x35000000
};

class PrimitiveCS
{
	DWORD		m_owningThread;

public:
	PrimitiveCS() : m_owningThread(0) {}

	PrimitiveCS *Enter();
	__forceinline void Leave() {m_owningThread = 0;}
};

class PrimitiveScopedLock
{
	PrimitiveCS		*m_cs;

public:
	PrimitiveScopedLock(PrimitiveCS &cs) : m_cs(&cs) {cs.Enter();}
	~PrimitiveScopedLock() {m_cs->Leave();}
};

class TESForm;
TESForm* __stdcall LookupFormByRefID(UInt32 refID);

int __vectorcall ifloor(float value);

int __vectorcall iceil(float value);

UInt32 __fastcall StrLen(const char* str);

void __fastcall MemZero(void* dest, UInt32 bsize);

char* __fastcall StrCopy(char* dest, const char* src);

char* __fastcall StrNCopy(char* dest, const char* src, UInt32 length);

char* __fastcall StrCat(char* dest, const char* src);

char __fastcall StrCompare(const char* lstr, const char* rstr);

void __fastcall StrToLower(char* str);

char* __fastcall SubStrCI(const char *srcStr, const char *subStr);

char* __fastcall SlashPos(const char *str);

char* __fastcall CopyString(const char* key);
char* __fastcall CopyString(const char* key, UInt32 length);

char* __fastcall IntToStr(char *str, int num);

UInt32 __fastcall StrHashCS(const char* inKey);

UInt32 __fastcall StrHashCI(const char* inKey);

float __vectorcall ASin(float x);
float __vectorcall ACos(float x);
float __vectorcall ATan2(float y, float x);

class SpinLock
{
	UInt32	owningThread;
	UInt32	enterCount;

public:
	SpinLock() : owningThread(0), enterCount(0) {}

	void Enter();
	void EnterSleep();
	void Leave();
};

#define GetRandomUInt(n) ThisStdCall<UInt32, UInt32>(0xAA5230, (void*)0x11C4180, n)

// From JIP
#define PS_DUP_1(a)	a, 0UL, 0UL, 0UL
#define PS_DUP_2(a)	a, a, 0UL, 0UL
#define PS_DUP_3(a)	a, a, a, 0UL
#define PS_DUP_4(a)	a, a, a, a

// From JIP
#define HEX(a) std::bit_cast<UInt32>(a)
#define UBYT(a) *((UInt8*)&a)
#define USHT(a) *((UInt16*)&a)
#define ULNG(a) *((UInt32*)&a)

// From JIP
extern const UInt32 kPackedValues[];

// From JIP
#define GET_PS(i)	((const __m128*)kPackedValues)[i]
#define GET_SS(i)	((const float*)kPackedValues)[i << 2]

// From JIP
#define PS_AbsMask			kPackedValues
#define PS_AbsMask0			kPackedValues+0x10
#define PS_FlipSignMask		kPackedValues+0x20
#define PS_FlipSignMask0	kPackedValues+0x30
#define PS_XYZ0Mask			kPackedValues+0x40
#define PD_AbsMask			kPackedValues+0x50
#define PD_FlipSignMask		kPackedValues+0x60

// From JIP
#define PS_Epsilon			kPackedValues+0x70
#define PS_V3_PId180		kPackedValues+0x80
#define PS_V3_180dPI		kPackedValues+0x90
#define PS_V3_PId2			kPackedValues+0xA0
#define PS_V3_PI			kPackedValues+0xB0
#define PS_V3_PIx2			kPackedValues+0xC0
#define PS_V3_Half			kPackedValues+0xD0
#define PS_V3_One			kPackedValues+0xE0
#define PS_HKUnitCnvrt		kPackedValues+0xF0

// From JIP
#define SS_1d1K				kPackedValues+0x100
#define SS_1d100			kPackedValues+0x104
#define SS_1d10				kPackedValues+0x108
#define SS_1d4				kPackedValues+0x10C
#define SS_3				kPackedValues+0x110
#define SS_10				kPackedValues+0x114
#define SS_100				kPackedValues+0x118

// From JIP
#define FltPId2		1.570796371F
#define FltPI		3.141592741F
#define FltPIx2		6.283185482F
#define FltPId180	0.01745329238F
#define Flt180dPI	57.29578018F
#define DblPId180	0.017453292519943295
#define Dbl180dPI	57.29577951308232

// From JIP
template <typename T_HashMap> class HashMapUtils
{
	using Entry = T_HashMap::Entry;
	using Bucket = T_HashMap::Bucket;

public:
	static void DumpLoads(const T_HashMap& map)
	{
		UInt32 loadsArray[0x40];
		__stosb((UInt8*)(loadsArray), 0, sizeof(loadsArray));
		UInt32 maxLoad = 0;
		for (Bucket* pBucket = map.GetBuckets(), *pEnd = map.End(); pBucket != pEnd; pBucket++)
		{
			UInt32 entryCount = pBucket->Size();
			loadsArray[entryCount]++;
			if (maxLoad < entryCount)
				maxLoad = entryCount;
		}
		PrintDebug("Size = %d\nBuckets = %d\n----------------\n", map.Size(), map.BucketCount());
		for (UInt32 iter = 0; iter <= maxLoad; iter++)
			if (loadsArray[iter]) PrintDebug("%d:\t%05d (%.4f%%)", iter, loadsArray[iter], 100.0 * (double)loadsArray[iter] / map.Size());
	}
};