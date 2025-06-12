#pragma once

#include "MainHeader.hpp"
#include "NiColor.hpp"
#include <NiParticleSystem.hpp>

namespace Overcharge
{
    //Overheating Code
    struct HeatInfo
    {
        HeatInfo();
        HeatInfo(float base, float perShot, float cooldown, float max);

        float   heatVal;
        float   baseHeatVal;
        float   heatPerShot;
        float   cooldownRate;
        float   maxHeat;

        inline void HeatOnFire()       //Responsible for heating a weapon up
        {
            heatVal += heatPerShot;         //Ticks up heatVal by the weapons defined heatPerShot value
        }
    };

    struct HeatFX
    {
        HeatFX();
        HeatFX(NiColor color1, NiColor color2, std::vector<const char*> names);

        NiColor                 currCol;
        NiColor                 startCol;
        NiColor                 targetCol;

        std::vector<const char*>  blockNames;

        inline NiColor SmoothShift(float currentHeat, float maxHeat) const
        {
            const float progress = std::clamp(currentHeat / maxHeat, 0.0f, 1.0f);
            return startCol.Shifted(targetCol, progress);
        }
    };

    struct HeatInstance
    {
        HeatInstance();
        HeatInstance(HeatInfo info, HeatFX fx);

        UInt32      actorForm;
        UInt32      weaponForm;

        HeatInfo    heat;
        HeatFX      fx;

        std::vector<NiAVObjectPtr>  targetBlocks;

        inline bool CompareForms(UInt32 actor, UInt32 weapon) const
        {
            return actorForm == actor && weaponForm == weapon;
        }
    };

    struct ParticleInstance
    {
        NiParticleSystem*               particle;
        std::pair<NiNode*, NiNode*>     nodePair;
    };

    struct ParticleInstance2
    {
        NiParticleSystem* particle;
        NiNode*    refNode;
    };

    extern std::vector<HeatInstance> activeWeapons;
    extern std::vector<Projectile*> activeProjectiles;

    HeatInstance MakeHeatFromTemplate(const HeatInstance& staticInst, const NiAVObjectPtr& sourceNode, UInt32 sourceRef, UInt32 weaponRef);

    void WeaponCooldown();
    void ParticleUpdater();
}
