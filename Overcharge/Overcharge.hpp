#pragma once

//Overcharge
#include "OverchargeConfig.hpp"

//Gamebryo
#include "NiParticleSystem.hpp"

namespace Overcharge
{
    constexpr float HOT_THRESHOLD = 100.0f;
    constexpr float COOL_THRESHOLD = 50.0f;
    constexpr float ERASE_DELAY = 5.0f;
    constexpr float COOLDOWN_DELAY = 0.5f;
    constexpr float CHARGE_THRESHOLD = 2.0f;

    constexpr UInt16 STOP_COOLDOWN_FLAGS = OCEffects_Overcharge | OCEffects_ChargeDelay;
    constexpr UInt16 STOP_FIRING_FLAGS = OCEffects_Overheat | OCEffects_Overcharge | OCEffects_ChargeDelay;

    //Active state of a weapon instance. These are live, hot and have their values updated regularly. 
    struct HeatState
    {
        //bIsActive is essentially a flag to queue either initialization and cleanup.
        bool    bIsActive             = false;       

        //0: Disabled, 1: Can Overheat (No Animation), 2: Can Overheat
        UInt8   iCanOverheat          = 2;               

        UInt8   uiAmmoUsed            = INVALID_U8;
        UInt8   uiProjectiles         = INVALID_U8;

        UInt16  uiDamage              = INVALID_U16;
        UInt16  uiCritDamage          = INVALID_U16;

        //uiTicksPassed serves as a generic progress counter for any time dependent features.
        UInt16  uiTicksPassed         = 0;

        //uiOCEffect represents the a state's currently *active* and applied OCEffects.
        UInt16  uiOCEffect            = 0;
        UInt32  uiObjectEffectID      = INVALID_U32;

        float   fAccuracy             = INVALID_F32;
        float   fFireRate             = INVALID_F32;
        float   fProjectileSpeed      = INVALID_F32;
        float   fProjectileSize       = INVALID_F32;
        float   fHeatPerShot          = INVALID_F32;
        float   fCooldownRate         = INVALID_F32;

        //Starting and Target Values are used for certain progress based mechanics such as flickering and charge delay. 
        float   fStartingVal          = 0.0f;
        float   fTargetVal            = 0.0f;

        //fHeatVal is the currently measured amount of heat (out of 100).
        float   fHeatVal              = 0.0f;

        HeatState() = default;

        HeatState(
            UInt8 ammo, UInt8 numProj, UInt16 dmg, UInt16 critDmg, UInt32 enchID,
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
            if (iCanOverheat > 0) {
                const bool overheating = IsOverheating();
                if (!overheating && IsHot())
                    uiOCEffect = OCEffects_Overheat;
                else if (overheating && IsCool())
                    uiOCEffect &= ~OCEffects_Overheat;
            }
            else uiOCEffect &= ~OCEffects_Overheat;
        }
    };

    struct OCBlock
    {
        UInt32 OCXFlags                 = 0x0;
        NiMaterialPropertyPtr matProp   = nullptr;
        NiAVObjectPtr target            = nullptr;
        float OCXThreshold              = 0.0f;
    };

    struct HeatFX
    {
        HeatFX();
        HeatFX(UInt32 col, std::vector<OCBlock> blocks);

        NiColor              currCol;

        BSSoundHandle        heatSoundHandle;
        BSSoundHandle        chargeSoundHandle;

        //targetBlocks contains all of the blocks on a weapon mesh that are controlled.
        std::vector<OCBlock> targetBlocks;
    };

    struct HeatData
    {
        HeatData(const HeatConfiguration* cfg);
        HeatData(HeatState heat, HeatFX visuals, const HeatConfiguration* config);

        HeatFX    fx;
        HeatState state;

        const HeatConfiguration* config;

        Actor*         rActor = nullptr;
        TESObjectWEAP* rWeap  = nullptr;
    };

    struct TempEffectGeometry
    {
        NiGeometryPtr pTempEffectGeom = nullptr;
        NiMaterialPropertyPtr ogMatProp = nullptr;
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
