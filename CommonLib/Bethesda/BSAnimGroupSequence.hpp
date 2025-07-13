#pragma once

#include "NiControllerSequence.hpp"

class TESAnimGroup;

class BSAnimGroupSequence : public NiControllerSequence {
public:
	BSAnimGroupSequence();
	virtual ~BSAnimGroupSequence();

	NIRTTI_ADDRESS(0x11C7D74);

	NiPointer<TESAnimGroup> spAnimGroup;

	BSAnimGroupSequence* Get3rdPersonCounterpart() const;
	float GetEaseInTime() const;
	float GetEaseOutTime() const;
};