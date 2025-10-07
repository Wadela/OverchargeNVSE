#pragma once
#include "MainHeader.hpp"
#include "NiColor.hpp"

namespace Overcharge
{
    //Overheating Code
    struct HeatInfo
    {
        HeatInfo(float base, float perShot, float cooldown, float max);

        float   heatVal;
        float   baseHeatVal;
        float   heatPerShot;
        float   cooldownRate;
        float   maxHeat;

        inline void HeatOnFire()       //Responsible for heating a weapon up
        {
            heatVal += heatPerShot;         //Ticks up heatVal by the weapons defined heatPerShot value
        }
    };

    struct HeatFX
    {
        HeatFX(NiColor color1, NiColor color2);

        NiColor     curCol;
        NiColor     startCol;
        NiColor     targetCol;

        NiNodePtr   sourceModel;
        NiNodePtr   muzzleFlashModel;
        NiNodePtr   projectileModel;
        NiNodePtr   impactModel;

        inline NiColor SmoothShift(const float currentHeat, const float maxHeat, const NiColor& startColor, const NiColor& endColor)
        {
            const float progress = std::clamp(currentHeat / maxHeat, 0.0f, 1.0f);

            return startColor.Shifted(endColor, progress);
        }
    };

    struct HeatActiveData
    {
        UInt32      actorForm;
        UInt32      weaponForm;

        HeatInfo*   heat;
        HeatFX*     fx;

        std::vector<const char*> blockNames;

        inline bool CompareForms(UInt32 actor, UInt32 weapon) const
        {
            return actorForm == actor && weaponForm == weapon;
        }
    };
}
