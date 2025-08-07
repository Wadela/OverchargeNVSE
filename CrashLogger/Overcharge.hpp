#pragma once

#include "MainHeader.hpp"
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
    };

    struct HeatData
    {
        HeatData(HeatState heat, HeatFX visuals, const HeatConfiguration* config);

        HeatFX      fx;
        HeatState   state;

        const HeatConfiguration* data;
    };

    UInt32  RGBtoUInt32(const NiColor& color);
    NiColor UInt32toRGB(const UInt32 color);
    NiColor UInt32toHSV(const UInt32 color);
    NiColor RGBtoHSV(const NiColor& color);                         //RGB -> Hue, Saturation, Value
    NiColor HSVtoRGB(const NiColor& hsv);                           //Hue, Saturation, Value -> RGB

    NiColor DesaturateRGB(NiColor rgb, float factor);
    NiColorA DesaturateRGBA(NiColorA rgba, float factor);

    NiColor SmoothColorShift(float currentHeat, UInt32 startCol, UInt32 targetCol);

    HeatData MakeHeatFromConfig(const HeatConfiguration* data, const NiAVObjectPtr& sourceNode);

}
