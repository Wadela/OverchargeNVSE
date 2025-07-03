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

    NiColor HeatFX::RGBtoHSV(const NiColor& color) const //RGB -> Hue, Saturation, Value
    {
        float r = color.r, g = color.g, b = color.b;
        float max = (std::max)({ r, g, b });
        float min = (std::min)({ r, g, b });
        float delta = max - min;

        NiColor out;
        out.b = max;

        if (delta == 0) {
            out.r = 0;
            out.g = 0;
        }
        else {
            out.g = delta / max;
            if (max == r)
                out.r = 60 * (fmod(((g - b) / delta), 6.0f));
            else if (max == g)
                out.r = 60 * (((b - r) / delta) + 2.0f);
            else
                out.r = 60 * (((r - g) / delta) + 4.0f);
            if (out.r < 0) out.r += 360;
        }
        return out;
    }

    NiColor HeatFX::HSVtoRGB(const NiColor& hsv) const //Hue, Saturation, Value -> RGB
    {
        float C = hsv.b * hsv.g;
        float X = C * (1 - fabs(fmod(hsv.r / 60.0f, 2) - 1));
        float m = hsv.b - C;

        float r, g, b;
        if (hsv.r < 60) { r = C; g = X; b = 0; }
        else if (hsv.r < 120) { r = X; g = C; b = 0; }
        else if (hsv.r < 180) { r = 0; g = C; b = X; }
        else if (hsv.r < 240) { r = 0; g = X; b = C; }
        else if (hsv.r < 300) { r = X; g = 0; b = C; }
        else { r = C; g = 0; b = X; }

        return NiColor(r + m, g + m, b + m);
    }

    NiColor HeatFX::SmoothShift(float currentHeat, float maxHeat) const
    {
        float progress = std::clamp(currentHeat / maxHeat, 0.0f, 1.0f);

        NiColor startHSV = RGBtoHSV(startCol);
        NiColor targetHSV = RGBtoHSV(targetCol);

        float hueDiff = targetHSV.r - startHSV.r;
        if (fabs(hueDiff) > 180.0f) {
            if (hueDiff > 0) hueDiff -= 360.0f;
            else hueDiff += 360.0f;
        }

        float interpHue = fmod(startHSV.r + hueDiff * progress + 360.0f, 360.0f);
        float interpSat = startHSV.g + (targetHSV.g - startHSV.g) * progress;
        float interpVal = startHSV.b + (targetHSV.b - startHSV.b) * progress;

        NiColor resultHSV = { interpHue, interpSat, interpVal };

        return HSVtoRGB(resultHSV);
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
    void HeatModProjectile(Projectile* proj, HeatInfo& heat)
    {
        if (!proj) return; 


    }
}

