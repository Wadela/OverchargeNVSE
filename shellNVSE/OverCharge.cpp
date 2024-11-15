#include "OverCharge.h"

int g_isOverheated = 0;

namespace Overcharge
{
    //Color Shift System
    HeatRGB HeatRGB::Blend(const HeatRGB& other, float ratio) const
    {
        float blendedRed = (this->heatRed * (1 - ratio)) + (other.heatRed * ratio);
        float blendedGreen = (this->heatGreen * (1 - ratio)) + (other.heatGreen * ratio);
        float blendedBlue = (this->heatBlue * (1 - ratio)) + (other.heatBlue * ratio);

        return HeatRGB{ blendedRed, blendedGreen, blendedBlue };
    }

    std::vector<HeatRGB> ColorGroup::BlendAll(float ratio)
    {
        std::vector<HeatRGB> blendedColors;
        for (size_t i = 0; i <= 6; ++i)
        {
            for (size_t j = i + 1; j <= 6; ++j)
            {
                // Use the blend function to combine colors[i] and colors[j]
                HeatRGB blendedColor = colorSet[i].Blend(colorSet[j], ratio);
                blendedColors.push_back(blendedColor);
            }
        }
        return blendedColors;
    }

    const ColorGroup* ColorGroup::GetColorSet(const char* colorName)
    {
        auto it = ColorGroup::colorMap.find(colorName);
        if (it != ColorGroup::colorMap.end())
        {
            return &it->second;
        }
    }

    const HeatRGB plasmaColorSet[] =
    {
        HeatRGB(1.000f, 0.486f, 0.655f),         //plasmaRed: #ff7ca7
        HeatRGB(1.000f, 0.698f, 0.486f),         //plasmaOrange: #ffb27c
        HeatRGB(1.000f, 1.000f, 0.486f),         //plasmaYellow: #ffff7c
        HeatRGB(0.655f, 1.000f, 0.486f),         //plasmaGreen: #a7ff7c
        HeatRGB(0.486f, 0.780f, 1.000f),         //plasmaBlue: #7cc7ff
        HeatRGB(0.655f, 0.486f, 1.000f),         //plasmaViolet: #a77cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //plasmaWhite: #e0f7ff
    };

    const HeatRGB laserColorSet[] =
    {
        HeatRGB(1.000f, 0.235f, 0.235f),         //laserRed: #ff3c3c
        HeatRGB(1.000f, 0.620f, 0.235f),         //laserOrange: #ff9e3c
        HeatRGB(1.000f, 1.000f, 0.235f),         //laserYellow: #ffff3c
        HeatRGB(0.192f, 0.965f, 0.325f),         //laserGreen: #31f752
        HeatRGB(0.227f, 0.667f, 0.965f),         //laserBlue: #3aaaf7
        HeatRGB(0.624f, 0.235f, 1.000f),         //laserViolet: #9f3cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //laserWhite: #e0f7ff
    };

    const HeatRGB flameColorSet[] =
    {
        HeatRGB(1.000f, 0.341f, 0.133f),         //flameRed: #ff5722
        HeatRGB(1.000f, 0.549f, 0.000f),         //flameOrange: #ff8c00 
        HeatRGB(1.000f, 0.925f, 0.235f),         //flameYellow: #ffeb3b
        HeatRGB(0.000f, 1.000f, 0.498f),         //flameGreen: #00ff7f
        HeatRGB(0.000f, 0.749f, 1.000f),         //flameBlue: #00bfff
        HeatRGB(0.608f, 0.000f, 1.000f),         //flameViolet: #9b00ff
        HeatRGB(0.961f, 0.961f, 0.961f)          //flameWhite: #f5f5f5
    };

    const HeatRGB zapColorSet[] =
    {
        HeatRGB(1.000f, 0.235f, 0.235f),         //zapRed: #ff3c3c
        HeatRGB(1.000f, 0.620f, 0.235f),         //zapOrange: #ff9e3c
        HeatRGB(1.000f, 1.000f, 0.486f),         //zapYellow: #ffff7c
        HeatRGB(0.655f, 1.000f, 0.486f),         //zapGreen: #a7ff7c
        HeatRGB(0.227f, 0.667f, 0.965f),         //zapBlue: #3aaaf7
        HeatRGB(0.655f, 0.486f, 1.000f),         //zapViolet: #a77cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //zapWhite: #e0f7ff
    };

    const ColorGroup ColorGroup::plasmaColors{ plasmaColorSet };
    const ColorGroup ColorGroup::laserColors{ laserColorSet };
    const ColorGroup ColorGroup::flameColors{ flameColorSet };
    const ColorGroup ColorGroup::zapColors{ zapColorSet };

    const std::unordered_map<std::string, ColorGroup> ColorGroup::colorMap = 
    {
        { "Plasma", ColorGroup::plasmaColors },
        { "Laser", ColorGroup::laserColors },
        { "Flame", ColorGroup::flameColors },
        { "Zap", ColorGroup::zapColors }
    };

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

