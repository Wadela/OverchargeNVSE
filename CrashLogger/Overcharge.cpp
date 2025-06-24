#include "Overcharge.hpp"

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

    HeatData::HeatData() :
        sourceID(0), heatedID(0), heat(), fx(),
        targetBlocks() {
    }

    HeatData::HeatData(HeatInfo info, HeatFX fx) :
        sourceID(0), heatedID(0), heat(info), fx(fx),
        targetBlocks() {
    }

    HeatData MakeHeatFromTemplate(const HeatData& staticData, const NiAVObjectPtr& sourceNode, UInt32 sourceRef, UInt32 weaponRef)
    {
        HeatData liveData = staticData;
        liveData.sourceID = sourceRef;
        liveData.heatedID = weaponRef;

        if (sourceNode)
        {
            for (const auto& name : liveData.fx.blockNames)
            {
                if (NiAVObjectPtr block = sourceNode->GetObjectByName(name))
                {
                    liveData.targetBlocks.emplace_back(block);
                }
            }
        }
        return liveData;
    }
}

