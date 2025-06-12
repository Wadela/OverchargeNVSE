#include "OverCharge.hpp"

namespace Overcharge
{
    HeatInfo::HeatInfo() :
        heatVal(0.0f), baseHeatVal(0.0f), heatPerShot(0.0f), cooldownRate(0.0f), maxHeat(1.0f) {
    }

    HeatInfo::HeatInfo(float base, float perShot, float cooldown, float max) :
        heatVal(base), baseHeatVal(base), heatPerShot(perShot), cooldownRate(cooldown), maxHeat(max) {
    }

    HeatFX::HeatFX() :
        currCol(0, 0, 0), startCol(0, 0, 0), targetCol(0, 0, 0),
        blockNames() {
    }

    HeatFX::HeatFX(NiColor color1, NiColor color2, std::vector<const char*> names) :
        currCol(color1), startCol(color1), targetCol(color2),
        blockNames(names) {
    }

    HeatInstance::HeatInstance() :
        actorForm(0), weaponForm(0), heat(), fx(), 
        targetBlocks() {
    }

    HeatInstance::HeatInstance(HeatInfo info, HeatFX fx) :
        actorForm(0), weaponForm(0), heat(info), fx(fx),
        targetBlocks() {
    }

    HeatInstance MakeHeatFromTemplate(const HeatInstance& staticInst, const NiAVObjectPtr& sourceNode, UInt32 sourceRef, UInt32 weaponRef)
    {
        HeatInstance liveInst = staticInst;
        liveInst.actorForm = sourceRef;
        liveInst.weaponForm = weaponRef;

        if (sourceNode)
        {
            for (const auto& name : liveInst.fx.blockNames)
            {
                if (NiAVObjectPtr block = sourceNode->GetObjectByName(name))
                {
                    liveInst.targetBlocks.emplace_back(block);
                }
            }
        }
        return liveInst;
    }
}

