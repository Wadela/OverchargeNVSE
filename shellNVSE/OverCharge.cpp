#include "OverCharge.h"

namespace Overcharge
{
    HeatRGB HeatRGB::blend(const HeatRGB& other, float ratio) const
    {
        float blendedRed = (this->heatRed * (1 - ratio)) + (other.heatRed * ratio);
        float blendedGreen = (this->heatGreen * (1 - ratio)) + (other.heatGreen * ratio);
        float blendedBlue = (this->heatBlue * (1 - ratio)) + (other.heatBlue * ratio);

        return HeatRGB{ blendedRed, blendedGreen, blendedBlue };
    }

    std::vector<HeatRGB> ColorGroup::blendAll(float ratio) 
    {
        std::vector<HeatRGB> blendedColors;
        for (size_t i = 0; i < colorSet.size(); ++i) 
        {
            for (size_t j = i + 1; j < colorSet.size(); ++j) 
            {
                // Use the blend function to combine colors[i] and colors[j]
                HeatRGB blendedColor = colorSet[i].blend(colorSet[j], ratio);
                blendedColors.push_back(blendedColor);
            }
        }
        return blendedColors;
    }

    const std::vector<HeatRGB> PlasmaColor::plasmaColorSet = 
    {
        HeatRGB(1.000f, 0.486f, 0.655f),         //plasmaRed: #ff7ca7
        HeatRGB(1.000f, 0.698f, 0.486f),         //plasmaOrange: #ffb27c
        HeatRGB(1.000f, 1.000f, 0.486f),         //plasmaYellow: #ffff7c
        HeatRGB(0.655f, 1.000f, 0.486f),         //plasmaGreen: #a7ff7c
        HeatRGB(0.486f, 0.780f, 1.000f),         //plasmaBlue: #7cc7ff
        HeatRGB(0.655f, 0.486f, 1.000f),         //plasmaViolet: #a77cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //plasmaWhite: #e0f7ff
    };

    const ColorGroup PlasmaColor::plasmaColors{ "Plasma", PlasmaColor::plasmaColorSet };
    const HeatRGB PlasmaColor::defaultPlasma = PlasmaColor::plasmaColorSet[3];

    const std::vector<HeatRGB> LaserColor::laserColorSet = 
    {
        HeatRGB(1.000f, 0.235f, 0.235f),         //laserRed: #ff3c3c
        HeatRGB(1.000f, 0.620f, 0.235f),         //laserOrange: #ff9e3c
        HeatRGB(1.000f, 1.000f, 0.235f),         //laserYellow: #ffff3c
        HeatRGB(0.192f, 0.965f, 0.325f),         //laserGreen: #31f752
        HeatRGB(0.227f, 0.667f, 0.965f),         //laserBlue: #3aaaf7
        HeatRGB(0.624f, 0.235f, 1.000f),         //laserViolet: #9f3cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //laserWhite: #e0f7ff
    };

    const ColorGroup LaserColor::laserColors{ "Laser", LaserColor::laserColorSet };
    const HeatRGB LaserColor::defaultLaser = LaserColor::laserColorSet[0];

    const std::vector<HeatRGB> FlameColor::flameColorSet = 
    {
        HeatRGB(1.000f, 0.341f, 0.133f),         //flameRed: #ff5722
        HeatRGB(1.000f, 0.549f, 0.000f),         //flameOrange: #ff8c00 
        HeatRGB(1.000f, 0.925f, 0.235f),         //flameYellow: #ffeb3b
        HeatRGB(0.000f, 1.000f, 0.498f),         //flameGreen: #00ff7f
        HeatRGB(0.000f, 0.749f, 1.000f),         //flameBlue: #00bfff
        HeatRGB(0.608f, 0.000f, 1.000f),         //flameViolet: #9b00ff
        HeatRGB(0.961f, 0.961f, 0.961f)          //flameWhite: #f5f5f5
    };

    const ColorGroup FlameColor::flameColors{ "Flame", FlameColor::flameColorSet };
    const HeatRGB FlameColor::defaultFlame = FlameColor::flameColorSet[1];

    const std::vector<HeatRGB> ZapColor::zapColorSet = 
    {
        HeatRGB(1.000f, 0.235f, 0.235f),         //zapRed: #ff3c3c
        HeatRGB(1.000f, 0.620f, 0.235f),         //zapOrange: #ff9e3c
        HeatRGB(1.000f, 1.000f, 0.486f),         //zapYellow: #ffff7c
        HeatRGB(0.655f, 1.000f, 0.486f),         //zapGreen: #a7ff7c
        HeatRGB(0.227f, 0.667f, 0.965f),         //zapBlue: #3aaaf7
        HeatRGB(0.655f, 0.486f, 1.000f),         //zapViolet: #a77cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //zapWhite: #e0f7ff
    };

    const ColorGroup ZapColor::zapColors{ "Zap", ZapColor::zapColorSet };
    const HeatRGB ZapColor::defaultZap = ZapColor::zapColorSet[4];
} 