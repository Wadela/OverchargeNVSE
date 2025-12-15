#pragma once

#include "Projectile.hpp"

class MissileProjectile : public Projectile {
public:
	enum ImpactResult {
		IR_NONE = 0,
		IR_DESTROY = 1,
		IR_BOUNCE = 2,
		IR_IMPALE = 3,
		IR_STICK = 4,
		IR_COUNT = 5,
	};

	ImpactResult	eImpactResult;
	bool			bWaitingToInitialize3D;
	float			fLifetime;
	float			fRotationAngle;

	void ProcessBounceNoCollision();
};

ASSERT_SIZE(MissileProjectile, 0x160);