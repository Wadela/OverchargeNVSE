#include "Overcharge.hpp"

namespace Overcharge
{
    HeatState::HeatState() :
        bIsWeaponLocked(false), uiOCEffect(0), uiAmmoUsed(0), uiProjectiles(0),
        uiAmmoThreshold(0), uiProjThreshold(0), uiEnchThreshold(0), uiOCEffectThreshold(0),
        uiDamage(0), uiCritDamage(0), uiObjectEffectID(0),
        fAccuracy(0.0f), fFireRate(0.0f), fProjectileSpeed(0.0f), fProjectileSize(0.0f),
        fHeatVal(0.0f), fHeatPerShot(0.0f), fCooldownRate(0.0f) {
    }

    HeatState::HeatState(
        UInt8 OCEffect, UInt8 ammo, UInt8 numProj,
        UInt8 ammoTH, UInt8 projTH, UInt8 enchTH, UInt8 effectTH,
        UInt16 dmg, UInt16 critDmg, UInt32 enchID,
        float projSpd, float projSize, float rof, float accuracy, float perShot, float cooldown) :

        bIsWeaponLocked(false), uiOCEffect(OCEffect), uiAmmoUsed(ammo), uiProjectiles(numProj),
        uiAmmoThreshold(ammoTH), uiProjThreshold(projTH), uiEnchThreshold(enchTH), uiOCEffectThreshold(effectTH),
        uiDamage(dmg), uiCritDamage(critDmg), uiObjectEffectID(enchID),
        fProjectileSpeed(projSpd), fProjectileSize(projSize), fFireRate(rof), fAccuracy(accuracy),
        fHeatVal(0.0f), fHeatPerShot(perShot), fCooldownRate(cooldown) {
    }

    UInt32 RGBtoUInt32(const NiColor& color)
    {
        UInt8 r = static_cast<UInt8>(color.r * 255.0f + 0.5f);
        UInt8 g = static_cast<UInt8>(color.g * 255.0f + 0.5f);
        UInt8 b = static_cast<UInt8>(color.b * 255.0f + 0.5f);

        return (b << 16) | (g << 8) | r;
    }

    NiColor RGBtoHSV(const NiColor& color) //RGB -> Hue, Saturation, Value
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

    NiColor HSVtoRGB(const NiColor& hsv)//Hue, Saturation, Value -> RGB
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

    NiColor UInt32toRGB(const UInt32 color)
    {
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = ((color) & 0xFF) / 255.0f;

        return NiColor(r, g, b);
    }

    NiColor UInt32toHSV(const UInt32 color)
    {
        NiColor RGB = UInt32toRGB(color);
        return RGBtoHSV(RGB);
    }

    NiColor DesaturateRGB(NiColor rgb, float factor)
    {
        NiColor hsv = RGBtoHSV(rgb);
        hsv.g *= (1.0f - factor);
        return HSVtoRGB(hsv);
    }

    NiColorA DesaturateRGBA(NiColorA rgba, float factor)
    {
        NiColor hsv = RGBtoHSV(NiColor(rgba.r, rgba.g, rgba.b));
        hsv.g *= (1.0f - factor);
        NiColor rgb = HSVtoRGB(hsv);
        return NiColorA(rgb.r, rgb.g, rgb.b, rgba.a);
    }

    NiColor SmoothColorShift(float currentHeat, UInt32 startCol, UInt32 targetCol)
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
        currCol(0, 0, 0),
        targetBlocks() {
    }

    HeatFX::HeatFX(UInt32 col, std::vector<NiAVObjectPtr> names) :
        currCol(UInt32toRGB(col)),
        targetBlocks(names) {
    }

    HeatData::HeatData(HeatState heat, HeatFX visuals, const HeatConfiguration* config) : state(heat), fx(visuals), data(config) {}

    HeatData MakeHeatFromConfig(const HeatConfiguration* data, const NiAVObjectPtr& sourceNode)
    {
        HeatState state;
        if (data) {
            state = HeatState(
                0,
                data->iMinAmmoUsed,
                data->iMinProjectiles,
                data->iAddAmmoThreshold,
                data->iAddProjectileThreshold,
                data->iObjectEffectThreshold,
                data->iOverchargeEffectThreshold,
                data->iMinDamage,
                data->iMinCritDamage,
                data->iObjectEffectID,
                data->iMinProjectileSpeedPercent,
                data->iMinProjectileSizePercent,
                data->fMinFireRate,
                data->fMinAccuracy,
                data->fHeatPerShot,
                data->fCooldownPerSecond
            );
        }

        std::vector<NiAVObjectPtr> blocks;
        if (sourceNode && data)
        {
            auto heatedNodes = SplitByDelimiter(data->sHeatedNodes, ',');
            for (const auto& name : heatedNodes)
            {
                if (NiAVObjectPtr block = sourceNode->GetObjectByName(name.c_str()))
                {
                    blocks.emplace_back(block);
                }
            }
        }

        HeatFX visuals(data ? data->iMinColor : 0, blocks);
        return HeatData(state, visuals, data);
    }

}

