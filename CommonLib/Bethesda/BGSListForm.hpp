#pragma once

#include "TESForm.hpp"

class BGSListForm : public TESForm {
public:
	BGSListForm();
	~BGSListForm();

	BSSimpleList<TESForm*>	kList;
	uint32_t				uiNumAddedObjects;

	uint32_t Count() const {
		return kList.ItemsInList();
	}

	TESForm* GetAt(uint32_t auID) const {
		auto pResult = kList.GetAt(auID);
		return pResult ? pResult->GetItem() : nullptr;
	}
};

ASSERT_SIZE(BGSListForm, 0x024);