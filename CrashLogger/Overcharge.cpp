#include "Overcharge.hpp"

namespace Overcharge
{
    HeatState::HeatState() :
        bIsOverheated(false), bEffectActive(false), 
        uiAmmoUsed(0), uiProjectiles(0), uiDamage(0), uiCritDamage(0), 
        fProjectileSpeed(0.0f), fProjectileSize(0.0f), fFireRate(0.0f), fAccuracy(0.0f),
        fHeatVal(0.0f), fHeatPerShot(0.0f), fCooldownRate(0.0f) {
    }

    HeatState::HeatState(
        UInt8 ammo, UInt8 numProj, UInt16 dmg, UInt16 critDmg,
        float projSpd, float projSize, float rof, float accuracy, float perShot, float cooldown) :

        bIsOverheated(false), bEffectActive(false),
        uiAmmoUsed(ammo), uiProjectiles(numProj), uiDamage(dmg), uiCritDamage(critDmg),
        fProjectileSpeed(projSpd), fProjectileSize(projSize), fFireRate(rof), fAccuracy(accuracy),
        fHeatVal(0.0f), fHeatPerShot(perShot), fCooldownRate(cooldown) {
    }

    UInt32 HeatFX::RGBtoUInt32(const NiColor& color) const
    {
        UInt8 r = static_cast<UInt8>(color.r * 255.0f + 0.5f);
        UInt8 g = static_cast<UInt8>(color.g * 255.0f + 0.5f);
        UInt8 b = static_cast<UInt8>(color.b * 255.0f + 0.5f);

        return (b << 16) | (g << 8) | r;
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

    NiColor HeatFX::UInt32toRGB(const UInt32 color) const
    {
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = ((color) & 0xFF) / 255.0f;

        return NiColor(r, g, b);
    }

    NiColor HeatFX::UInt32toHSV(const UInt32 color) const
    {
        NiColor RGB = UInt32toRGB(color);
        return RGBtoHSV(RGB);
    }

    NiColor HeatFX::SmoothShift(float currentHeat) const
    {
        float progress = std::clamp(currentHeat / 100.0f, 0.0f, 1.0f);

        NiColor startHSV = UInt32toHSV(startCol);
        NiColor targetHSV = UInt32toHSV(targetCol);

        float hueDiff = targetHSV.r - startHSV.r;
        if (fabs(hueDiff) > 180.0f) {
            if (hueDiff > 0) hueDiff -= 360.0f;
            else hueDiff += 360.0f;
        }

        float interpHue = std::fmod(startHSV.r + hueDiff * progress, 360.0f);
        if (interpHue < 0.0f) interpHue += 360.0f;
        float interpSat = startHSV.g + (targetHSV.g - startHSV.g) * progress;
        float interpVal = startHSV.b + (targetHSV.b - startHSV.b) * progress;

        NiColor resultHSV = { interpHue, interpSat, interpVal };

        return HSVtoRGB(resultHSV);
    }

    HeatFX::HeatFX() :
        currCol(0, 0, 0), startCol(0), targetCol(0),
        targetBlocks() {
    }

    HeatFX::HeatFX(UInt32 col1, UInt32 col2, std::vector<NiAVObjectPtr> names) :
        currCol(UInt32toRGB(col1)), startCol(col1), targetCol(col2),
        targetBlocks(names) {
    }

    HeatData::HeatData(HeatState heat, HeatFX visuals, HeatConfiguration& config) : state(heat), fx(visuals), data(config) {}

    HeatData MakeHeatFromConfig(HeatConfiguration& data, const NiAVObjectPtr& sourceNode)
    {
        HeatState state = HeatState(data.iMinAmmoUsed, data.iMinProjectiles, data.iMinDamage, data.iMinCritDamage, data.iMinProjectileSpeedPercent, data.iMinProjectileSizePercent, data.fMinFireRate, data.fMinFireRate, data.fHeatPerShot, data.fCooldownPerSecond);

        std::vector<NiAVObjectPtr> blocks;

        if (sourceNode)
        {
            for (const auto& name : data.sHeatedNodes)
            {
                if (NiAVObjectPtr block = sourceNode->GetObjectByName(name.c_str()))
                {
                    blocks.emplace_back(block);
                }
            }
        }
        HeatFX visuals(data.iMinColor, data.iMaxColor, blocks);
        return HeatData(state, visuals, data); 
    }

}

