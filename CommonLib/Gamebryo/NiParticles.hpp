#pragma once

#include "NiGeometry.hpp"

NiSmartPointer(NiParticles);

class NiParticles : public NiGeometry {
public:
	NiParticles();
	virtual ~NiParticles();

	CREATE_OBJECT(NiParticles, 0xA9B150);
	NIRTTI_ADDRESS(0x11f5f64);
};

ASSERT_SIZE(NiParticles, 0xC4)