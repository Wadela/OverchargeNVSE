#pragma once
#include "MainHeader.h"
#include <unordered_map>
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

        HeatRGB() : heatRed(0.0f), heatGreen(0.0f), heatBlue(0.0f) {}
        HeatRGB(float r, float g, float b) : heatRed(r), heatGreen(g), heatBlue(b) {}

        HeatRGB ColorShift(const HeatRGB& target, float increment)
        {
            HeatRGB result; 

            result.heatRed = std::clamp(heatRed + increment * (target.heatRed - heatRed), 0.0f, 1.0f);
            result.heatGreen = std::clamp(heatGreen + increment * (target.heatGreen - heatGreen), 0.0f, 1.0f);
            result.heatBlue = std::clamp(heatBlue + increment * (target.heatBlue - heatBlue), 0.0f, 1.0f);

            return result;
        }

    };

    struct ColorTiers
    {

        HeatRGB redTier;
        HeatRGB orangeTier;
        HeatRGB yellowTier;
        HeatRGB greenTier;
        HeatRGB blueTier;
        HeatRGB violetTier;
        HeatRGB whiteTier;
        HeatRGB defaultTier;

        ColorTiers(HeatRGB red, HeatRGB orange, HeatRGB yellow, HeatRGB green, HeatRGB blue, HeatRGB violet, HeatRGB white, HeatRGB defaultColor) :
            redTier(red), orangeTier(orange), yellowTier(yellow), greenTier(green), blueTier(blue), violetTier(violet), whiteTier(white), defaultTier(defaultColor)
        {}

    };

    extern std::unordered_map<UInt32, ColorTiers> DefaultColorMap;
    void InitDefault()
    {
        ColorTiers plasmaColor
        {

            HeatRGB(1.000f, 0.486f, 0.655f),         //redTier Color: #ff7ca7
            HeatRGB(1.000f, 0.698f, 0.486f),         //orangeTier Color: #ffb27c
            HeatRGB(1.000f, 1.000f, 0.486f),         //yellowTier Color: #ffff7c
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c
            HeatRGB(0.486f, 0.780f, 1.000f),         //blueTier Color: #7cc7ff
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(0.655f, 1.000f, 0.486f)          //defaultTier (Green) Color: #a7ff7c

        };

        ColorTiers laserColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.235f),         //yellowTier Color: #ffff3c
            HeatRGB(0.192f, 0.965f, 0.325f),         //greenTier Color: #31f752
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.624f, 0.235f, 1.000f),         //violetTier Color: #9f3cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(1.000f, 0.235f, 0.235f)          //defaultTier (Red) Color: #ff3c3c

        };

        ColorTiers laserGreenColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.235f),         //yellowTier Color: #ffff3c
            HeatRGB(0.192f, 0.965f, 0.325f),         //greenTier Color: #31f752
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.624f, 0.235f, 1.000f),         //violetTier Color: #9f3cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(0.192f, 0.965f, 0.325f)          //defaultTier (Green) Color: #31f752

        };

        ColorTiers laserOrangeColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.235f),         //yellowTier Color: #ffff3c
            HeatRGB(0.192f, 0.965f, 0.325f),         //greenTier Color: #31f752
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.624f, 0.235f, 1.000f),         //violetTier Color: #9f3cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(1.000f, 0.620f, 0.235f)          //defaultTier (Orange) Color: #ff9e3c

        };

        ColorTiers laserYellowColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.235f),         //yellowTier Color: #ffff3c
            HeatRGB(0.192f, 0.965f, 0.325f),         //greenTier Color: #31f752
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.624f, 0.235f, 1.000f),         //violetTier Color: #9f3cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(1.000f, 1.000f, 0.235f)          //defaultTier (Yellow) Color: #ff9e3c

        };

        ColorTiers laserBlueColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.235f),         //yellowTier Color: #ffff3c
            HeatRGB(0.192f, 0.965f, 0.325f),         //greenTier Color: #31f752
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.624f, 0.235f, 1.000f),         //violetTier Color: #9f3cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(0.227f, 0.667f, 0.965f)          //defaultTier (Blue) Color: #ff9e3c

        };

        ColorTiers flameColor
        {

            HeatRGB(1.000f, 0.341f, 0.133f),         //redTier Color: #ff5722
            HeatRGB(1.000f, 0.549f, 0.000f),         //orangeTier Color: #ff8c00 
            HeatRGB(1.000f, 0.925f, 0.235f),         //yellowTier Color: #ffeb3b
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c - Unused
            HeatRGB(0.000f, 0.749f, 1.000f),         //blueTier Color: #00bfff
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff - Unused
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff - Unused
            HeatRGB(1.000f, 0.549f, 0.000f)          //defaultTier (Orange) Color: #ff8c00
             
        };

        ColorTiers flameBlueColor
        {

            HeatRGB(1.000f, 0.341f, 0.133f),         //redTier Color: #ff5722
            HeatRGB(1.000f, 0.549f, 0.000f),         //orangeTier Color: #ff8c00 
            HeatRGB(1.000f, 0.925f, 0.235f),         //yellowTier Color: #ffeb3b
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c - Unused
            HeatRGB(0.000f, 0.749f, 1.000f),         //blueTier Color: #00bfff
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff - Unused
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff - Unused
            HeatRGB(0.000f, 0.749f, 1.000f)          //defaultTier (Blue) Color: #00bfff

        };

        ColorTiers flameYellowColor
        {

            HeatRGB(1.000f, 0.341f, 0.133f),         //redTier Color: #ff5722
            HeatRGB(1.000f, 0.549f, 0.000f),         //orangeTier Color: #ff8c00 
            HeatRGB(1.000f, 0.925f, 0.235f),         //yellowTier Color: #ffeb3b
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c - Unused
            HeatRGB(0.000f, 0.749f, 1.000f),         //blueTier Color: #00bfff
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff - Unused
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff - Unused
            HeatRGB(1.000f, 0.925f, 0.235f)          //defaultTier (Yellow) Color: #00bfff

        };

        ColorTiers zapColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.486f),         //yellowTier Color: #ffff7c
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(0.227f, 0.667f, 0.965f)          //defaultTier (Blue) Color: #3aaaf7

        };

        ColorTiers zapOrangeColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.486f),         //yellowTier Color: #ffff7c
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(1.000f, 0.620f, 0.235f)          //defaultTier (Orange) Color: #ff9e3c

        };

        ColorTiers zapWhiteColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.486f),         //yellowTier Color: #ffff7c
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(0.878f, 0.969f, 1.000f)          //defaultTier (White) Color: #e0f7ff

        };

        ColorTiers zapVioletColor
        {

            HeatRGB(1.000f, 0.235f, 0.235f),         //redTier Color: #ff3c3c
            HeatRGB(1.000f, 0.620f, 0.235f),         //orangeTier Color: #ff9e3c
            HeatRGB(1.000f, 1.000f, 0.486f),         //yellowTier Color: #ffff7c
            HeatRGB(0.655f, 1.000f, 0.486f),         //greenTier Color: #a7ff7c
            HeatRGB(0.227f, 0.667f, 0.965f),         //blueTier Color: #3aaaf7
            HeatRGB(0.655f, 0.486f, 1.000f),         //violetTier Color: #a77cff
            HeatRGB(0.878f, 0.969f, 1.000f),         //whiteTier Color: #e0f7ff
            HeatRGB(0.655f, 0.486f, 1.000f)          //defaultTier (Violet) Color: #a77cff

        };

        //Initialize defaults for plasma weapons
        DefaultColorMap.emplace(kPlasmaRifle, plasmaColor); 
        //DefaultColorMap.emplace(kPlasmaRifleQ35, plasmaColor);
        //DefaultColorMap.emplace(kPlasmaRifleVanG, plasmaColor);
        //DefaultColorMap.emplace(kPlasmaPistol, plasmaColor);  
        //DefaultColorMap.emplace(kPlasmaPistolGRA, plasmaColor); 
        //DefaultColorMap.emplace(kPlasmaCaster, plasmaColor);  
        //DefaultColorMap.emplace(kPlasmaCasterSmittyS, plasmaColor);
        //DefaultColorMap.emplace(kPlasmaDefender, plasmaColor);
        //DefaultColorMap.emplace(kPlasmaDefenderGRA, plasmaColor);
        //DefaultColorMap.emplace(kMultiplasRifle, plasmaColor);

        //Initialize defaults for laser weapons
        //DefaultColorMap.emplace(kLaserPistol, laserColor); 
        //DefaultColorMap.emplace(kLaserPistolGRA, laserColor); 
        //DefaultColorMap.emplace(kLaserPistolPewPew, laserColor);
        //DefaultColorMap.emplace(kLaserRifle, laserColor); 
        //DefaultColorMap.emplace(kLaserRifleVanG, laserColor); 
        //DefaultColorMap.emplace(kLaserRCW, laserColor); 
        //DefaultColorMap.emplace(kGatlingLaser, laserColor); 
        //DefaultColorMap.emplace(kTriBeamLaserRifle, laserColor); 
        //DefaultColorMap.emplace(kTriBeamLaserRifleGRA, laserColor); 

        //Initialize defaults for green laser weapons 
        //DefaultColorMap.emplace(kLaserRifleAER14, laserGreenColor); 
        //DefaultColorMap.emplace(kGatlingLaserSprtelW, laserGreenColor); 

        //Initialize defaults for orange laser weapons
        //DefaultColorMap.emplace(kRechargerPistol, laserOrangeColor); 
        //DefaultColorMap.emplace(kRechargerRifle, laserOrangeColor);
        //DefaultColorMap.emplace(kLaserPistolPewPew, laserOrangeColor);

        //Initialize defaults for blue laser weapons
        //DefaultColorMap.emplace(kLAER, laserBlueColor);
        //DefaultColorMap.emplace(kLAERElijah, laserBlueColor);

        //Initialize defaults for fire weapons
        //DefaultColorMap.emplace(kFlamer, flameColor); 
        //DefaultColorMap.emplace(kIncinerator, flameColor); 
        //DefaultColorMap.emplace(kIncineratorHeavy, flameColor); 
        //DefaultColorMap.emplace(kShishKebab, flameColor); 

        //Initialize defaults for blue fire weapons
        //DefaultColorMap.emplace(kFlamerCleansingF, flameBlueColor);

        //Initialize defaults for yellow fire weapons
        //DefaultColorMap.emplace(kShishKebabGehenna, flameYellowColor);

        //Initialize defaults for electric weapons
        //DefaultColorMap.emplace(kArcWelder, zapColor);
        //DefaultColorMap.emplace(kTeslaCannon, zapColor);
        //DefaultColorMap.emplace(kPulseGun, zapColor);
        //DefaultColorMap.emplace(kProtonAxe, zapColor);

        //Initialize defaults for orange electric weapons
        //DefaultColorMap.emplace(kTeslaCannonBeatonP, zapOrangeColor);
        //DefaultColorMap.emplace(kTeslaCannonElijah, zapOrangeColor);

        //Initialize defaults for white electric weapons
        //DefaultColorMap.emplace(kAlienBlaster, zapWhiteColor); 

        auto iter = DefaultColorMap.find(kPlasmaRifle);
        ColorTiers Default = iter->second;

        HeatRGB currentColor = Default.defaultTier; 


    }

    auto iter = DefaultColorMap.find(kPlasmaRifle);
    ColorTiers Default = iter->second;

    HeatRGB currentColor = Default.defaultTier; 

}