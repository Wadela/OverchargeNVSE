#pragma once

//Overcharge
#include "OverchargeConfig.hpp"

//Gamebryo
#include "NiParticleSystem.hpp"

namespace Overcharge
{
    constexpr float HOT_THRESHOLD = 100.0f;
    constexpr float COOL_THRESHOLD = 20.0f;
    constexpr float ERASE_DELAY = 5.0f;
    constexpr float COOLDOWN_DELAY = 0.5f;
    constexpr float CHARGE_THRESHOLD = 1.2f;

    constexpr UInt16 STOP_COOLDOWN_FLAGS = OCEffects_Overcharge | OCEffects_ChargeDelay;
    constexpr UInt16 STOP_FIRING_FLAGS = OCEffects_Overheat | OCEffects_Overcharge | OCEffects_ChargeDelay;

    //Overheating Code
    struct HeatState
    {
        bool    bIsActive           = true;
        bool    bCanOverheat        = true;

        UInt8   uiAmmoUsed          = 0xFF;
        UInt8   uiProjectiles       = 0xFF;
        UInt8   uiAmmoThreshold     = 0xFF;
        UInt8   uiProjThreshold     = 0xFF;
        UInt8   uiEnchThreshold     = 0xFF;
        UInt8   uiOCEffectThreshold = 0xFF;

        UInt16  uiDamage            = 0xFFFF;
        UInt16  uiCritDamage        = 0xFFFF;

        UInt16  uiTicksPassed       = 0xFFFF;
        UInt16  uiOCEffect          = 0xFFFF;

        UInt32  uiObjectEffectID    = 0xFFFFFF;

        float   fAccuracy           = -1;
        float   fFireRate           = -1;
        float   fProjectileSpeed    = -1;
        float   fProjectileSize     = -1;

        float   fHeatVal            = 0.0f;
        float   fHeatPerShot        = -1;
        float   fCooldownRate       = -1;

        HeatState() = default;

        HeatState(
            UInt8 ammo, UInt8 numProj,
            UInt8 ammoTH, UInt8 projTH, UInt8 enchTH, UInt8 effectTH,
            UInt16 dmg, UInt16 critDmg, UInt16 OCEffect, UInt32 enchID,
            float accuracy, float rof, float projSpd, float projSize,
            float perShot, float cooldown);

        HeatState(const HeatConfiguration& config);


        inline void HeatOnFire()
        {
            fHeatVal += fHeatPerShot;
            uiTicksPassed = 0;
        }

        inline bool IsHot() const { return fHeatVal >= HOT_THRESHOLD; }
        inline bool IsCool() const { return fHeatVal <= COOL_THRESHOLD; }
        inline bool IsOverheating() const { return (uiOCEffect & OCEffects_Overheat) != 0; }

        inline void UpdateOverheat()
        {
            if (bCanOverheat == true)
            {
                const bool overheating = IsOverheating();

                if (!overheating && IsHot())
                    uiOCEffect |= OCEffects_Overheat;
                else if (overheating && IsCool())
                    uiOCEffect &= ~OCEffects_Overheat;
            } 
            else uiOCEffect &= ~OCEffects_Overheat;
        }
    };

    struct HeatFX
    {
        HeatFX();
        HeatFX(UInt32 col, std::vector<std::pair<UInt32, NiAVObjectPtr>> names);

        NiColor                    currCol;
        NiMaterialPropertyPtr      objMatProp;
        NiMaterialPropertyPtr      fxMatProp;

        std::vector<std::pair<UInt32, NiAVObjectPtr>> targetBlocks;
    };

    struct HeatData
    {
        HeatData(const HeatConfiguration* cfg);
        HeatData(HeatState heat, HeatFX visuals, const HeatConfiguration* config);

        Actor* rActor = nullptr;
        TESObjectWEAP* rWeap = nullptr;

        HeatFX    fx;
        HeatState state;

        const HeatConfiguration* config;
    };

    UInt32  RGBtoUInt32(const NiColor& color);
    NiColor UInt32toRGB(const UInt32 color);
    NiColor UInt32toHSV(const UInt32 color);
    NiColor RGBtoHSV(const NiColor& color);                         //RGB -> Hue, Saturation, Value
    NiColor HSVtoRGB(const NiColor& hsv);                           //Hue, Saturation, Value -> RGB

    NiColor DesaturateRGB(NiColor rgb, float factor);
    NiColorA DesaturateRGBA(NiColorA rgba, float factor);

    NiColor SmoothColorShift(float currentHeat, UInt32 startCol, UInt32 targetCol);
}
