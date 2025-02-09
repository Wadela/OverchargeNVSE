#include "OverCharge.hpp"

int g_isOverheated = 0;

namespace Overcharge
{
    const NiColor plasmaColorSet[] = 
    {
        NiColor(1.000f, 0.486f, 0.655f),         //plasmaRed: #ff7ca7
        NiColor(1.000f, 0.698f, 0.486f),         //plasmaOrange: #ffb27c
        NiColor(1.000f, 1.000f, 0.486f),         //plasmaYellow: #ffff7c
        NiColor(0.655f, 1.000f, 0.486f),         //plasmaGreen: #a7ff7c
        NiColor(0.486f, 0.780f, 1.000f),         //plasmaBlue: #7cc7ff
        NiColor(0.655f, 0.486f, 1.000f),         //plasmaViolet: #a77cff
        NiColor(0.878f, 0.969f, 1.000f)          //plasmaWhite: #e0f7ff
    };

    const NiColor laserColorSet[] =
    {
        NiColor(1.000f, 0.235f, 0.235f),         //laserRed: #ff3c3c
        NiColor(1.000f, 0.620f, 0.235f),         //laserOrange: #ff9e3c
        NiColor(1.000f, 1.000f, 0.235f),         //laserYellow: #ffff3c
        NiColor(0.192f, 0.965f, 0.325f),         //laserGreen: #31f752
        NiColor(0.227f, 0.667f, 0.965f),         //laserBlue: #3aaaf7
        NiColor(0.624f, 0.235f, 1.000f),         //laserViolet: #9f3cff
        NiColor(0.878f, 0.969f, 1.000f)          //laserWhite: #e0f7ff
    };

    const NiColor flameColorSet[] =
    {
        NiColor(1.000f, 0.341f, 0.133f),         //flameRed: #ff5722
        NiColor(1.000f, 0.549f, 0.000f),         //flameOrange: #ff8c00 
        NiColor(1.000f, 0.925f, 0.235f),         //flameYellow: #ffeb3b
        NiColor(0.000f, 1.000f, 0.498f),         //flameGreen: #00ff7f
        NiColor(0.000f, 0.749f, 1.000f),         //flameBlue: #00bfff
        NiColor(0.608f, 0.000f, 1.000f),         //flameViolet: #9b00ff
        NiColor(0.961f, 0.961f, 0.961f)          //flameWhite: #f5f5f5
    };

    const NiColor zapColorSet[] =
    {
        NiColor(1.000f, 0.235f, 0.235f),         //zapRed: #ff3c3c
        NiColor(1.000f, 0.620f, 0.235f),         //zapOrange: #ff9e3c
        NiColor(1.000f, 1.000f, 0.486f),         //zapYellow: #ffff7c
        NiColor(0.655f, 1.000f, 0.486f),         //zapGreen: #a7ff7c
        NiColor(0.227f, 0.667f, 0.965f),         //zapBlue: #3aaaf7
        NiColor(0.655f, 0.486f, 1.000f),         //zapViolet: #a77cff
        NiColor(0.878f, 0.969f, 1.000f)          //zapWhite: #e0f7ff
    };

    const NiColor* plasmaColorArray[7] = { &plasmaColorSet[0], &plasmaColorSet[1], &plasmaColorSet[2], &plasmaColorSet[3], &plasmaColorSet[4], &plasmaColorSet[5], &plasmaColorSet[6] };
    const NiColor* laserColorArray[7] = { &laserColorSet[0], &laserColorSet[1], &laserColorSet[2], &laserColorSet[3], &laserColorSet[4], &laserColorSet[5], &laserColorSet[6] };
    const NiColor* flameColorArray[7] = { &flameColorSet[0], &flameColorSet[1], &flameColorSet[2], &flameColorSet[3], &flameColorSet[4], &flameColorSet[5], &flameColorSet[6] };
    const NiColor* zapColorArray[7] = { &zapColorSet[0], &zapColorSet[1], &zapColorSet[2], &zapColorSet[3], &zapColorSet[4], &zapColorSet[5], &zapColorSet[6] };

    const ColorGroup plasmaColors(plasmaColorArray);
    const ColorGroup laserColors(laserColorArray);
    const ColorGroup flameColors(flameColorArray);
    const ColorGroup zapColors(zapColorArray);

    static constexpr std::pair<const char*, const ColorGroup*> colorMap[] = 
    {
        {"Plasma", &plasmaColors},
        {"Laser", &laserColors},
        {"Flame", &flameColors},
        {"Zap", &zapColors},
    };

    const ColorGroup* ColorGroup::GetColorSet(const char* colorName)
    {
        for (const auto& [key, group] : colorMap)
        {
            if (strcmp(key, colorName) == 0)
                return group;
        }
        return &plasmaColors; // Default fallback
    }

    //Overheating System
    void WeaponHeat::HeatOnFire()       //Responsible for heating a weapon up
    {
        float maxHeat = 300.0f;

        heatVal += heatPerShot;         //Ticks up heatVal by the weapons defined heatPerShot value

        if (heatVal >= maxHeat)         //If heatVal reaches maximum heat threshold --> Weapon overheats
        {
            g_isOverheated = 1;         //When g_isOverheated == 1, Weapon does not fire.
        }
    }

}

