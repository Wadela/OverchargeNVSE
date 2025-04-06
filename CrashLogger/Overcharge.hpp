#pragma once
#include "MainHeader.hpp"
#include "NiColor.hpp"

namespace Overcharge
{
    //Overheating Code
    struct WeaponHeat
    {
        float baseHeatVal;
        float heatVal;
        float heatPerShot;
        float cooldownRate;
        float maxHeat;

        bool isOverheated;

        WeaponHeat(float base, float perShot, float cooldown, float max, bool overheat = false) :
            baseHeatVal(base), heatVal(base), heatPerShot(perShot), cooldownRate(cooldown), maxHeat(max), isOverheated(overheat) {}

        inline void HeatOnFire()       //Responsible for heating a weapon up
        {
            heatVal += heatPerShot;         //Ticks up heatVal by the weapons defined heatPerShot value
        }
    };

    struct HeatedWeaponData
    {
        UInt32 actorForm;
        UInt32 weaponForm;

        WeaponHeat heatData;
        NiColor currentColor;
        NiColor startingColor;
        NiColor targetColor;

        NiNodePtr sourceModel;
        NiNodePtr muzzleFlashModel;
        NiNodePtr projectileModel; 
        NiNodePtr impactModel; 

        NiMaterialPropertyPtr sourceMatProp;
        NiMaterialPropertyPtr muzzleMatProp;
        NiMaterialPropertyPtr projMatProp;
        NiMaterialPropertyPtr impactMatProp;

        std::vector<const char*> blockNames;

        HeatedWeaponData(
            WeaponHeat heat, NiColor color1, NiColor color2, std::vector<const char*> blocks) :
            actorForm(0), weaponForm(0), heatData(heat), currentColor(color1), startingColor(color1), targetColor(color2), 
            sourceModel(nullptr), muzzleFlashModel(nullptr), projectileModel(nullptr), impactModel(nullptr),
            sourceMatProp(nullptr), muzzleMatProp(nullptr), projMatProp(nullptr), impactMatProp(nullptr), blockNames(blocks) {}  

        inline bool CompareForms(UInt32 actorID, UInt32 weaponID) const 
        {
            return actorForm == actorID && weaponForm == weaponID; 
        }

        inline NiColor SmoothShift(const float currentHeat, const float maxHeat, const NiColor& startColor, const NiColor& endColor) 
        {
            const float progress = std::clamp(currentHeat / maxHeat, 0.0f, 1.0f);

            return startColor.Shifted(endColor, progress);
        }
    };

    extern std::vector<HeatedWeaponData> activeWeapons;
    extern std::vector<Projectile*> activeProjectiles;

    void WeaponCooldown();
    void ParticleUpdater();
}
