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
    //Color Changing Code
    struct HeatRGB
    {
        float heatRed;
        float heatGreen;
        float heatBlue;

        HeatRGB(float r, float g, float b) : heatRed{ r }, heatGreen{ g }, heatBlue{ b } {}

        HeatRGB Blend(const HeatRGB& other, float ratio) const;
    };

    struct ColorGroup
    {
        std::string colorType;
        const HeatRGB* colorSet;

        ColorGroup(const std::string& type, const HeatRGB* colorSet) : colorType(type), colorSet(colorSet) {}

        std::vector<HeatRGB> BlendAll(float ratio);

        static const ColorGroup* GetColorSet(const char* colorName)
        {
            auto it = ColorGroup::colorMap.find(colorName);

            if (it != colorMap.end())
            {
                return &it->second;
            }
            return nullptr;         //We don't know if this is a valid color group or not.
        }

    private:

        static const std::unordered_map<std::string, ColorGroup> colorMap;

        static const ColorGroup plasmaColors;
        static const ColorGroup laserColors;
        static const ColorGroup flameColors;
        static const ColorGroup zapColors;

        static const std::unordered_map<std::string, ColorGroup> InitializeColorMap()
        {
            return
            {
                {"Plasma", plasmaColors},
                {"Laser", laserColors},
                {"Flame", flameColors},
                {"Zap", zapColors}
            };
        }
    };

    struct ColorShift
    {
        HeatRGB startColor;
        HeatRGB targetColor;
        float currentRatio;
        float incRatio;

        ColorShift() :
            startColor(0.0f, 0.0f, 0.0f),
            targetColor(0.0f, 0.0f, 0.0f),
            currentRatio(0.0f),
            incRatio(0.0f) {}

        ColorShift(const HeatRGB& start, const HeatRGB& end, float step) :
            startColor(start), targetColor(end), currentRatio(0.0f), incRatio(step) {}

        HeatRGB Shift()
        {
            if (currentRatio > 1.0f)
            {
                currentRatio = 1.0f;
            }

            HeatRGB blendedColor = startColor.Blend(targetColor, currentRatio);

            currentRatio += incRatio;

            return blendedColor;
        }

        void ResetColor()
        {
            currentRatio = 0.0f;
        }
    };

    //Overheating Code
    struct WeaponHeat
    {
        double heatVal;
        double heatPerShot;
        double cooldownRate;

        WeaponHeat(double initialHeatVal, double heatPerShotVal, double cooldownRateVal) :
            heatVal(initialHeatVal), heatPerShot(heatPerShotVal), cooldownRate(cooldownRateVal) {}

        void HeatOnFire();
    };

    extern std::unordered_map<UInt32, WeaponHeat> heatedWeapons;
    extern std::vector<const char*> blockNames;

    void WeaponCooldown();
}
