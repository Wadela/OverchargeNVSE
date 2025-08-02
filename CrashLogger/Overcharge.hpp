#pragma once

#include "MainHeader.hpp"
#include "NiColor.hpp"
#include <NiParticleSystem.hpp>
#include "OverchargeConfig.hpp"

namespace Overcharge
{
    //Overheating Code
    struct HeatState
    {
        HeatState();
        HeatState(
        UInt8 ammo, UInt8 numProj, UInt16 dmg, UInt16 critDmg, 
        float projSpd, float projSize, float rof, float accuracy, float perShot, float cooldown);

        bool    bIsOverheated;
        bool    bEffectActive;

        UInt8   uiAmmoUsed;
        UInt8   uiProjectiles;
        UInt16  uiDamage;
        UInt16  uiCritDamage;

        float   fProjectileSpeed;
        float   fProjectileSize;
        float   fFireRate;
        float   fAccuracy;

        float   fHeatVal;
        float   fHeatPerShot;
        float   fCooldownRate;

        inline void HeatOnFire()
        {
            fHeatVal += fHeatPerShot;
        }
    };

    struct HeatFX
    {
        HeatFX();
        HeatFX(UInt32 col1, UInt32 col2, std::vector<NiAVObjectPtr> names);

        NiColor     currCol;
        UInt32      startCol;
        UInt32      targetCol;

        NiMaterialPropertyPtr       matProp;

        std::vector<NiAVObjectPtr>  targetBlocks;

        UInt32  RGBtoUInt32(const NiColor& color) const;
        NiColor UInt32toRGB(const UInt32 color) const;
        NiColor UInt32toHSV(const UInt32 color) const;
        NiColor RGBtoHSV(const NiColor& color) const;                   //RGB -> Hue, Saturation, Value
        NiColor HSVtoRGB(const NiColor& hsv) const;                     //Hue, Saturation, Value -> RGB
        NiColor SmoothShift(float currentHeat) const;
    };

    struct HeatData
    {
        HeatData(HeatState heat, HeatFX visuals, const HeatConfiguration* config);

        HeatFX      fx;
        HeatState   state;

        const HeatConfiguration* data;
    };

    extern std::unordered_map<UInt64, std::shared_ptr<HeatData>> activeWeapons;

    HeatData MakeHeatFromConfig(const HeatConfiguration* data, const NiAVObjectPtr& sourceNode);
    void WeaponCooldown();
}
