#pragma once
#include "BaseFormComponent.hpp"

class TESImageSpaceModifier;

// 0x8
class TESImageSpaceModifiableForm : public BaseFormComponent {
public:
	TESImageSpaceModifiableForm();
	~TESImageSpaceModifiableForm();

	TESImageSpaceModifier* pModifier;
};
static_assert(sizeof(TESImageSpaceModifiableForm) == 0x8);