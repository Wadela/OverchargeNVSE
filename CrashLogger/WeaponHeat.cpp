#include "WeaponHeat.hpp"

namespace Overcharge
{
    HeatInfo::HeatInfo(float base, float perShot, float cooldown, float max) 
        : baseHeatVal(base), heatPerShot(perShot), cooldownRate(cooldown), maxHeat(max) {}

    HeatFX::HeatFX(NiColor color1, NiColor color2)
        : actorForm(0), weaponForm(0),
          curCol(color1), startCol(color1), targetCol(color2),
          sourceModel(nullptr), muzzleFlashModel(nullptr), projectileModel(nullptr), impactModel(nullptr) {}
}

