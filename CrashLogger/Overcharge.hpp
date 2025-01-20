#pragma once

#include "MainHeader.hpp"


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
            int stepCount = abs(color2 - color1);
            float heatRatio = min(heatVal / 300.0f, 1.0f);
            int currentStep = static_cast<int>(heatRatio * stepCount);
            currentStep = std::clamp(currentStep, 0, stepCount);

            bool forward = color2 > color1;

            int currentIndex = forward ? color1 + currentStep : color1 - currentStep;
            int nextIndex = forward ? currentIndex + 1 : currentIndex - 1;
            nextIndex = std::clamp(nextIndex, 0, 6);

            HeatRGB currentColor = set->colorSet[currentIndex];
            HeatRGB nextColor = set->colorSet[nextIndex];

            float localHeatRatio = (heatRatio * stepCount) - currentStep;

            HeatRGB blendedColor = currentColor.Blend(nextColor, localHeatRatio);

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
        TESForm* weaponRef;
        TESObjectREFR* projectileRef;
        TESObjectREFR* actorRef;
        NiMaterialProperty* matProperty;
        ColorShift colorData;
        WeaponHeat heatData;

        WeaponData(TESForm* weap, TESObjectREFR* proj, TESObjectREFR* actor, NiMaterialProperty* matProp, ColorShift weaponColor, WeaponHeat weaponHeat) :
            weaponRef(weap), projectileRef(proj), actorRef(actor), matProperty(matProp), colorData(weaponColor), heatData(weaponHeat) {}

        std::vector<const char*> blockNames;
    };

    extern std::unordered_map<UInt32, WeaponData> heatedWeapons;

    void WeaponCooldown();
}
