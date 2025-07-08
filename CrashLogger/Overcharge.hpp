#pragma once

#include "MainHeader.hpp"
#include "NiColor.hpp"
#include <NiParticleSystem.hpp>

namespace Overcharge
{
    //Overheating Code
    struct HeatInfo
    {
        HeatInfo();
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
        HeatFX();
        HeatFX(NiColor color1, NiColor color2, std::vector<const char*> names);

        NiColor                 currCol;
        NiColor                 startCol;
        NiColor                 targetCol;

        std::vector<const char*>  blockNames;

        NiColor RGBtoHSV(const NiColor& color) const;  //RGB -> Hue, Saturation, Value
        NiColor HSVtoRGB(const NiColor& hsv) const;    //Hue, Saturation, Value -> RGB
        NiColor SmoothShift(float currentHeat, float maxHeat) const;
    };

    struct HeatData
    {
        HeatData();
        HeatData(HeatInfo info, HeatFX fx);

        UInt32      sourceID;
        UInt32      heatedID;

        HeatInfo    heat;
        HeatFX      fx;

        std::vector<NiAVObjectPtr> targetBlocks; 

        inline bool CompareForms(UInt32 actor, UInt32 weapon) const
        {
            return sourceID == actor && heatedID == weapon;
        }
    };

    struct HeatModifiers
    {
        UInt8   numAmmo;
        UInt8   numProjectiles;
        UInt8   projSpeed; 
        UInt8   critEffect;

        UInt16  damage;
        UInt16  critDamage;

        float   critChance;
        float   minSpread;
        float   maxSpread;
    };

    extern std::unordered_map<UInt64, std::shared_ptr<HeatData>> activeWeapons;

    HeatData MakeHeatFromTemplate(const HeatData& staticInst, const NiAVObjectPtr& sourceNode, UInt32 sourceRef, UInt32 weaponRef);

    void HeatModProjectile(Projectile* proj, HeatInfo& heat);
    void WeaponCooldown();
}
