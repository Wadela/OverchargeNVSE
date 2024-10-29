#pragma once
#include "MainHeader.h"
#include <unordered_map>
#include <vector>
#include "Hooks_Gameplay.h"
#include <unordered_set>

#include "CommandTable.h"
#include "Utilities.h"
#include "GameObjects.h"
#include <array>
#include "GameUI.h"
#include "SafeWrite.h"

extern int g_isOverheated;

namespace Overcharge
{
    enum OverChargeWeaps
    {
        kUnknownEnergyWeapon = 0,
        kAlienBlaster, 
        kArcWelder,
        kFlamer,
        kFlamerCleansingF,
        kGatlingLaser,
        kGatlingLaserSprtelW,
        kGaussRifle,
        kGaussRifleYCS,
        kHoloRifle,
        kIncinerator,
        kIncineratorHeavy,
        kLAER,
        kLAERElijah,
        kLaserPistol,
        kLaserPistolGRA,
        kLaserPistolPewPew,
        kLaserRCW,
        kLaserRifle,
        kLaserRifleAER14,
        kLaserRifleVanG,
        kMultiplasRifle,
        kPlasmaCaster,
        kPlasmaCasterSmittyS,
        kPlasmaDefender,
        kPlasmaDefenderGRA,
        kPlasmaPistol,
        kPlasmaPistolGRA,
        kPlasmaRifle,
        kPlasmaRifleQ35,
        kPlasmaRifleVanG,
        kPulseGun,
        kRechargerPistol,
        kRechargerPistolMFHA,
        kRechargerRifle,
        kSonicEmitterGabeB,
        kSonicEmitterOperaS,
        kSonicEmitterRev,
        kSonicEmitterRoboS,
        kSonicEmitterTar,
        kTeslaCannon,
        kTeslaCannonBeatonP,
        kTeslaCannonElijah,
        kTriBeamLaserRifle,
        kTriBeamLaserRifleGRA,
        kShishKebab,
        kShishKebabGehenna,
        kProtonAxe,
        kProtonicInversalAxe
    };

    //Color Changing Code
    struct HeatRGB
    {
        float heatRed;
        float heatGreen;
        float heatBlue;

        HeatRGB(float r, float g, float b) : heatRed{ r }, heatGreen{ g }, heatBlue{ b } {}

        HeatRGB blend(const HeatRGB& other, float ratio) const;
    };

    struct ColorGroup
    {
        std::string colorType;
        std::vector<HeatRGB> colorSet;

        ColorGroup(const std::string& type, const std::vector<HeatRGB>& set) : colorType(type), colorSet(set) {}

        std::vector<HeatRGB> blendAll(float ratio);
    };

    struct PlasmaColor
    {
        static const std::vector<HeatRGB> plasmaColorSet;

        static const ColorGroup plasmaColors;
        static const HeatRGB defaultPlasma;
    };

    struct LaserColor
    {
        static const std::vector<HeatRGB> laserColorSet;

        static const ColorGroup laserColors;
        static const HeatRGB defaultLaser;
    };

    struct FlameColor
    {
        static const std::vector<HeatRGB> flameColorSet;

        static const ColorGroup flameColors;
        static const HeatRGB defaultFlame;
    };

    struct ZapColor
    {
        static const std::vector<HeatRGB> zapColorSet;

        static const ColorGroup zapColors;
        static const HeatRGB defaultZap;
    };

    struct ColorShift
    {
        HeatRGB startColor;
        HeatRGB targetColor;
        float currentRatio;
        float incRatio;

        ColorShift(const HeatRGB& start, const HeatRGB& end, float step) : 
            startColor(start), targetColor(end), currentRatio(0.0f), incRatio(step) {} 

        HeatRGB Shift()
        {
            if (currentRatio > 1.0f)
            {
                currentRatio = 1.0f;
            }

            HeatRGB blendedColor = startColor.blend(targetColor, currentRatio);

            currentRatio += incRatio;

            return blendedColor;
        }

        void resetColor()
        {
            currentRatio = 0.0f;
        }
    };

    //Overheating Code
    struct WeaponHeat
    {
        float heatVal;
        float heatPerShot; 
        float cooldownRate;

        WeaponHeat(float initialHeatVal, float heatPerShotVal, float cooldownRateVal) :
            heatVal(initialHeatVal), heatPerShot(heatPerShotVal), cooldownRate(cooldownRateVal) {} 

        void HeatOnFire();
    };

    extern std::vector<WeaponHeat> heatedWeapons; 

    void WeaponCooldown();
}