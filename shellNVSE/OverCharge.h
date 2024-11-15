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
        const HeatRGB* colorSet;

        ColorGroup(const HeatRGB* colorSet) : colorSet(colorSet) {}

        std::vector<HeatRGB> BlendAll(float ratio);

        static const ColorGroup* GetColorSet(const char* colorName); 

        static const std::unordered_map<std::string, ColorGroup> colorMap;

        static const ColorGroup plasmaColors;
        static const ColorGroup laserColors;
        static const ColorGroup flameColors;
        static const ColorGroup zapColors;
    };

    struct ColorShift
    {
        const ColorGroup* colorType;
        const HeatRGB startColor;
        const HeatRGB targetColor;
        int startIndex;
        int targetIndex;

        ColorShift() :
            colorType(0),
            startColor(0.0f, 0.0f, 0.0f),
            targetColor(0.0f, 0.0f, 0.0f),
            startIndex(0),
            targetIndex(0) {}

        ColorShift(const ColorGroup* selectedCG, const HeatRGB& start, const HeatRGB& end, int setIndex1, int setIndex2) :
            colorType(selectedCG), startColor(start), targetColor(end), startIndex(setIndex1), targetIndex(setIndex2) {}

        HeatRGB Shift(float heatVal, int color1, int color2, const ColorGroup* set)
        {
            int stepCount = (color2 - color1);

            float heatRatio = min(heatVal / 300.0f, 1.0f);

            int currentStep = static_cast<int>(heatRatio * abs(stepCount));
            currentStep = std::clamp(currentStep, 0, abs(stepCount));

            HeatRGB startColor = set->colorSet[color1];
            HeatRGB targetColor = set->colorSet[color2];


            HeatRGB currentColor = (stepCount > 0)
                ? set->colorSet[color1 + currentStep]
                : set->colorSet[color1 - currentStep]; 

            HeatRGB blendedColor = startColor.Blend(currentColor, heatRatio);

            return blendedColor;
        }
    };

    //Overheating Code
    struct WeaponHeat
    {
        double baseHeatVal;
        double heatVal;
        double heatPerShot;
        double cooldownRate;

        WeaponHeat(double initialHeatVal, double heatPerShotVal, double cooldownRateVal) :
            baseHeatVal(initialHeatVal), heatVal(initialHeatVal), heatPerShot(heatPerShotVal), cooldownRate(cooldownRateVal) {}

        void HeatOnFire();
    };

    struct WeaponData
    {
        ColorShift colorData;
        WeaponHeat heatData; 
        TESObjectREFR* actorRef;

        WeaponData(ColorShift weaponColor, WeaponHeat weaponHeat, TESObjectREFR* actor) :
            colorData(weaponColor), heatData(weaponHeat), actorRef(actor) {}

        std::vector<const char*> blockNames;
    };

    extern std::unordered_map<UInt32, WeaponData> heatedWeapons;

    void WeaponCooldown();
}
