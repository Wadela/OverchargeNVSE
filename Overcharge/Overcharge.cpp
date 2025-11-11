#include "Overcharge.hpp"

namespace Overcharge
{
    HeatState::HeatState(
        UInt8 ammoTH, UInt8 projTH, UInt8 enchTH, UInt8 effectTH,
        UInt8 ammo, UInt8 numProj, UInt16 dmg, UInt16 critDmg, UInt32 enchID,
        float accuracy, float rof, float projSpd, float projSize,
        float perShot, float cooldown
    ) :
        bIsActive(false), bCanOverheat(true),
        uiAmmoThreshold(ammoTH), uiProjThreshold(projTH), uiEnchThreshold(enchTH), uiOCEffectThreshold(effectTH),
        uiAmmoUsed(ammo), uiProjectiles(numProj), uiDamage(dmg), uiCritDamage(critDmg), 
        uiTicksPassed(0), uiOCEffect(0), uiObjectEffectID(enchID),
        fAccuracy(accuracy), fFireRate(rof), fProjectileSpeed(projSpd), fProjectileSize(projSize),
        fHeatPerShot(perShot), fCooldownRate(cooldown), fHeatVal(0.0f), fStartingVal(0.0f), fTargetVal(0.0f) {
    }

    HeatState::HeatState(const HeatConfiguration& config)
        : HeatState(
            config.iAddAmmoThreshold,
            config.iAddProjectileThreshold,
            config.iObjectEffectThreshold,
            config.iOverchargeEffectThreshold,
            config.iMinAmmoUsed,
            config.iMinProjectiles,
            config.fMinDamage,
            config.fMinCritDamage,
            config.iObjectEffectID,
            config.fMinSpread,
            config.fMinFireRate,
            config.fMinProjectileSpeed,
            config.fMinProjectileSize,
            config.fHeatPerShot,
            config.fCooldownPerSecond
        )
    {
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
        float r = color.r;
        float g = color.g;
        float b = color.b;

        float max = r;
        float min = r;

        if (g > max) max = g;
        else if (g < min) min = g;

        if (b > max) max = b;
        else if (b < min) min = b;

        float delta = max - min;
        NiColor out;
        out.b = max;

        if (delta == 0.0f) {
            out.r = 0.0f;  
            out.g = 0.0f;   
            return out;
        }

        out.g = delta / max;
        float hue;

        if (max == r) {
            hue = (g - b) / delta;
        }
        else if (max == g) {
            hue = 2.0f + (b - r) / delta;
        }
        else {
            hue = 4.0f + (r - g) / delta;
        }

        hue *= 60.0f;
        if (hue < 0.0f) hue += 360.0f;
        out.r = hue;
        return out;
    }

    NiColor HSVtoRGB(const NiColor& hsv) //Hue, Saturation, Value -> RGB
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
        float progress = std::clamp(currentHeat / HOT_THRESHOLD, 0.0f, 1.0f);

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

    HeatFX::HeatFX(UInt32 col, std::vector<OCBlock> blocks) :
        currCol(UInt32toRGB(col)),
        targetBlocks(std::move(blocks)) {
    }

    HeatData::HeatData(HeatState heat, HeatFX visuals, const HeatConfiguration* cfg) : state(heat), fx(visuals), config(cfg) {}

    HeatData::HeatData(const HeatConfiguration* cfg): 
        state(cfg ? *cfg : HeatState()),
        fx(cfg ? cfg->iMinColor : 0, {}),
        config(cfg) {
    }
}

