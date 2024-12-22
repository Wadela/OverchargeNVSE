#pragma once

#include "BSExtraData.hpp"
#include "BSSimpleList.hpp"
#include "BSPointer.hpp"

class TESObjectREFR;
class TESObjectCELL;
class ExtraRefractionProperty;
class ExtraSayTopicInfoOnceADay;

class ExtraDataList : public BaseExtraList {
public:
	static ExtraDataList* Create(BSExtraData* xBSData = NULL);

	BSSimpleList<BSPointer<TESObjectREFR>*>* GetReflectedRefs() const;
	ExtraRefractionProperty* GetRefractionProperty() const;
	ExtraSayTopicInfoOnceADay* GetExtraSayTopicInfoOnceADayExtra() const;

	void SetPersistentCell(TESObjectCELL* apCell);
};

ASSERT_SIZE(ExtraDataList, 0x20);