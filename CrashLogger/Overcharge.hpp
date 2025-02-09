#pragma once
#include "MainHeader.hpp"
#include "NiColor.hpp"

extern int g_isOverheated;
namespace Overcharge
{
    //Color Changing Code
    struct ColorGroup
    {
        const NiColor* colorSet[7];

        ColorGroup(const NiColor* colors[])
        {
            for (size_t i = 0; i < 7; ++i)
            {
                colorSet[i] = colors[i];
            }
        }

        // Safe lookup with bounds checking 
        const NiColor* GetColor(size_t index) const
        {
            return colorSet[index < 7 ? index : 6]; // Bound checking to 6 (max index)
        }

        static const ColorGroup* GetColorSet(const char* colorName);
    };

    struct ColorShift
    {
        const ColorGroup* colorType;
        const NiColor* startColor;
        const NiColor* targetColor;
        int startIndex;
        int targetIndex;

        ColorShift(const ColorGroup* selectedCG, const NiColor* start, const NiColor* end, int setIndex1, int setIndex2) :
            colorType(selectedCG), startColor(start), targetColor(end), startIndex(setIndex1), targetIndex(setIndex2) {}

        NiColor StepShift(float heatVal, int color1, int color2, const ColorGroup* set)
        {
            int stepCount = abs(color2 - color1);
            float heatRatio = min(heatVal / 300.0f, 1.0f);
            int currentStep = static_cast<int>(heatRatio * stepCount);
            currentStep = std::clamp(currentStep, 0, stepCount);

            bool forward = color2 > color1;

            int currentIndex = forward ? color1 + currentStep : color1 - currentStep;
            int nextIndex = forward ? currentIndex + 1 : currentIndex - 1;
            nextIndex = std::clamp(nextIndex, 0, 6);


            const NiColor* currentColor = set->GetColor(currentIndex); 
            const NiColor* nextColor = set->GetColor(nextIndex);

            float localHeatRatio = (heatRatio * stepCount) - currentStep;

            NiColor blendedColor = currentColor->Shifted(*nextColor, localHeatRatio); 

            return blendedColor;
        }

        NiColor SmoothShift(const float time, const float period, const NiColor& startColor)
        {
            // Normalize time to avoid overflow if time grows large
            const float normalizedTime = fmod(time, period);

            // Use sine wave functions, with the starting color defining phase offsets
            const float r = (sin((normalizedTime / period) + startColor.r * 2 * std::numbers::pi) + 1) / 2;
            const float g = (sin((normalizedTime / period) + startColor.g * 2 * std::numbers::pi + 2 * std::numbers::pi / 3) + 1) / 2;
            const float b = (sin((normalizedTime / period) + startColor.b * 2 * std::numbers::pi + 4 * std::numbers::pi / 3) + 1) / 2;

            // Return the RGB color as a NiColor struct
            return { r, g, b };
        }

    };

    //Overheating Code
    struct WeaponHeat
    {
        float baseHeatVal;
        float heatVal;
        float heatPerShot;
        float cooldownRate;

        WeaponHeat(float initialHeatVal, float heatPerShotVal, float cooldownRateVal) :
            baseHeatVal(initialHeatVal), heatVal(initialHeatVal), heatPerShot(heatPerShotVal), cooldownRate(cooldownRateVal) {}

        void HeatOnFire();
    };

    struct WeaponData
    {
        NiAVObject* meshData;
        ColorShift colorData;
        WeaponHeat heatData;

        WeaponData(NiAVObject* mesh, ColorShift color, WeaponHeat heat) :
            meshData(mesh), colorData(color), heatData(heat) {
        }

        std::vector<const char*> blockNames;
    };

    extern std::unordered_map<UInt32, WeaponData> heatedWeapons;

    void WeaponCooldown();
}
