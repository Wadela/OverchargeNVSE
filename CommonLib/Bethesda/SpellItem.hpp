#pragma once

#include "MagicItemForm.hpp"

class SpellItem : public MagicItemForm {
public:
	SpellItem();
	~SpellItem();

	virtual void	Endian();

	enum EnumType : UInt32
	{
		kType_ActorEffect = 0,
		kType_Disease,
		kType_Power,
		kType_LesserPower,
		kType_Ability,
		kType_Poison,
		kType_Addiction = 10,
	};

	struct Data {
		EnumType				eType;
		UInt32					uiCost;
		UInt32					uiLevel;
		UInt8					ucSpellFlags;
	};

	Data kData;
};

//ASSERT_SIZE(SpellItem, 0x44);