#pragma once

#include "TESFullName.hpp"
#include "EffectItem.hpp"

class TESFile;

// 0x1C
class MagicItem :
	public TESFullName,
	public EffectItemList
{
public:
	MagicItem();
	~MagicItem();

	class Data {
	public:
		SInt32		iCostOverride;
		Bitfield32	uiFlags;
	};

	virtual bool					IsAutoCalc() const; // Name overlap with TESObject
	virtual void					SetAutoCalc(bool abVal);
	virtual UInt32					GetSpellType() const;
	virtual bool					Unk_07();
	virtual bool					Unk_08();
	virtual UInt32					GetChunkID() const;
	virtual Data*					GetData() const;
	virtual UInt32					GetDataSize();
	virtual bool					CompareMagicItemData(MagicItem* apItem);
	virtual void					CopyMagicItemData(MagicItem* apItem);
	virtual void					SaveMagicItemComponents();
	virtual void					Endian();
	virtual void					LoadMagicItemChunk(TESFile* apFile, UInt32 aeChunkID);

	//uint32_t GetMagicItemFormID() const;
};
//ASSERT_SIZE(MagicItem, 0x1C);