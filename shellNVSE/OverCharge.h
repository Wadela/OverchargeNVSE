#pragma once
#include "MainHeader.h"
#include <unordered_map>
#include <vector>

namespace Overcharge
{
    enum OverChargeWeaps
    {

        kAlienBlaster = 0x004322,
        kArcWelder = 0x00766B,
        kFlamer = 0x00432D,
        kFlamerCleansingF = 0x00080B,
        kGatlingLaser = 0x00432E,
        kGatlingLaserSprtelW = 0x000803,
        kGaussRifle = 0x15837B,
        kGaussRifleYCS = 0x15B38D,
        kHoloRifle = 0x0092EF,
        kIncinerator = 0x0906DA,
        kIncineratorHeavy = 0x0E2BFC,
        kLAER = 0x00B9CF,
        kLAERElijah = 0x015DD3,
        kLaserPistol = 0x004335,
        kLaserPistolGRA = 0x00084D,
        kLaserPistolPewPew = 0x103B1D,
        kLaserRCW = 0x09073B,
        kLaserRifle = 0x004336,
        kLaserRifleAER14 = 0x1479B3,
        kLaserRifleVanG = 0x1251CD,
        kMultiplasRifle = 0x121168,
        kPlasmaCaster = 0x0906CF,
        kPlasmaCasterSmittyS = 0x00080A,
        kPlasmaDefender = 0x090727,
        kPlasmaDefenderGRA = 0x000853,
        kPlasmaPistol = 0x004343,
        kPlasmaPistolGRA = 0x000855,
        kPlasmaRifle = 0x004344,
        kPlasmaRifleQ35 = 0x0E6064,
        kPlasmaRifleVanG = 0x1251CC,
        kPulseGun = 0x090A6B,
        kRechargerPistol = 0x09071F,
        kRechargerPistolMFHA = 0x000802,
        kRechargerRifle = 0x121154,
        kSonicEmitterGabeB = 0x00E37E,
        kSonicEmitterOperaS = 0x00E1B8,
        kSonicEmitterRev = 0x00E381,
        kSonicEmitterRoboS = 0x00E386,
        kSonicEmitterTar = 0x00E389,
        kTeslaCannon = 0x0E2BEC,
        kTeslaCannonBeatonP = 0x15A47F,
        kTeslaCannonElijah = 0x01199E,
        kTriBeamLaserRifle = 0x0E2BF4,
        kTriBeamLaserRifleGRA = 0x000854,
        kShishKebab = 0x00434E,
        kShishKebabGehenna = 0x00080F,
        kProtonAxe = 0x016A4B,
        kProtonicInversalAxe = 0x009701
    };

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
}