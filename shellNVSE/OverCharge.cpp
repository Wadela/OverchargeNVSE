#include "OverCharge.h"

namespace DisablePlayerControlsAlt
{
    extern std::map<ModID, flags_t> g_disabledFlagsPerMod;   
    extern flags_t g_disabledControls = 0;  // Definition and initialization of global variable

}

void __cdecl HUDMainMenu::UpdateVisibilityState(signed int hudState)
{
    return CdeclCall(0x771700, hudState);
}

UInt32 originalAddress;

namespace Overcharge
{
    HeatRGB HeatRGB::blend(const HeatRGB& other, float ratio) const
    {
        float blendedRed = (this->heatRed * (1 - ratio)) + (other.heatRed * ratio);
        float blendedGreen = (this->heatGreen * (1 - ratio)) + (other.heatGreen * ratio);
        float blendedBlue = (this->heatBlue * (1 - ratio)) + (other.heatBlue * ratio);

        return HeatRGB{ blendedRed, blendedGreen, blendedBlue };
    }

    std::vector<HeatRGB> ColorGroup::blendAll(float ratio) 
    {
        std::vector<HeatRGB> blendedColors;
        for (size_t i = 0; i < colorSet.size(); ++i) 
        {
            for (size_t j = i + 1; j < colorSet.size(); ++j) 
            {
                // Use the blend function to combine colors[i] and colors[j]
                HeatRGB blendedColor = colorSet[i].blend(colorSet[j], ratio);
                blendedColors.push_back(blendedColor);
            }
        }
        return blendedColors;
    }

    const std::vector<HeatRGB> PlasmaColor::plasmaColorSet = 
    {
        HeatRGB(1.000f, 0.486f, 0.655f),         //plasmaRed: #ff7ca7
        HeatRGB(1.000f, 0.698f, 0.486f),         //plasmaOrange: #ffb27c
        HeatRGB(1.000f, 1.000f, 0.486f),         //plasmaYellow: #ffff7c
        HeatRGB(0.655f, 1.000f, 0.486f),         //plasmaGreen: #a7ff7c
        HeatRGB(0.486f, 0.780f, 1.000f),         //plasmaBlue: #7cc7ff
        HeatRGB(0.655f, 0.486f, 1.000f),         //plasmaViolet: #a77cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //plasmaWhite: #e0f7ff
    };

    const ColorGroup PlasmaColor::plasmaColors{ "Plasma", PlasmaColor::plasmaColorSet };
    const HeatRGB PlasmaColor::defaultPlasma = PlasmaColor::plasmaColorSet[3];

    const std::vector<HeatRGB> LaserColor::laserColorSet = 
    {
        HeatRGB(1.000f, 0.235f, 0.235f),         //laserRed: #ff3c3c
        HeatRGB(1.000f, 0.620f, 0.235f),         //laserOrange: #ff9e3c
        HeatRGB(1.000f, 1.000f, 0.235f),         //laserYellow: #ffff3c
        HeatRGB(0.192f, 0.965f, 0.325f),         //laserGreen: #31f752
        HeatRGB(0.227f, 0.667f, 0.965f),         //laserBlue: #3aaaf7
        HeatRGB(0.624f, 0.235f, 1.000f),         //laserViolet: #9f3cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //laserWhite: #e0f7ff
    };

    const ColorGroup LaserColor::laserColors{ "Laser", LaserColor::laserColorSet };
    const HeatRGB LaserColor::defaultLaser = LaserColor::laserColorSet[0];

    const std::vector<HeatRGB> FlameColor::flameColorSet = 
    {
        HeatRGB(1.000f, 0.341f, 0.133f),         //flameRed: #ff5722
        HeatRGB(1.000f, 0.549f, 0.000f),         //flameOrange: #ff8c00 
        HeatRGB(1.000f, 0.925f, 0.235f),         //flameYellow: #ffeb3b
        HeatRGB(0.000f, 1.000f, 0.498f),         //flameGreen: #00ff7f
        HeatRGB(0.000f, 0.749f, 1.000f),         //flameBlue: #00bfff
        HeatRGB(0.608f, 0.000f, 1.000f),         //flameViolet: #9b00ff
        HeatRGB(0.961f, 0.961f, 0.961f)          //flameWhite: #f5f5f5
    };

    const ColorGroup FlameColor::flameColors{ "Flame", FlameColor::flameColorSet };
    const HeatRGB FlameColor::defaultFlame = FlameColor::flameColorSet[1];

    const std::vector<HeatRGB> ZapColor::zapColorSet = 
    {
        HeatRGB(1.000f, 0.235f, 0.235f),         //zapRed: #ff3c3c
        HeatRGB(1.000f, 0.620f, 0.235f),         //zapOrange: #ff9e3c
        HeatRGB(1.000f, 1.000f, 0.486f),         //zapYellow: #ffff7c
        HeatRGB(0.655f, 1.000f, 0.486f),         //zapGreen: #a7ff7c
        HeatRGB(0.227f, 0.667f, 0.965f),         //zapBlue: #3aaaf7
        HeatRGB(0.655f, 0.486f, 1.000f),         //zapViolet: #a77cff
        HeatRGB(0.878f, 0.969f, 1.000f)          //zapWhite: #e0f7ff
    };

    const ColorGroup ZapColor::zapColors{ "Zap", ZapColor::zapColorSet };
    const HeatRGB ZapColor::defaultZap = ZapColor::zapColorSet[4];

    UInt32 g_PreventAttackDetour;
    bool __fastcall MaybePreventPlayerAttacking2(Actor* player, void* edx, UInt32 animGroupId)
    {
        if ((DisablePlayerControlsAlt::g_disabledControls & DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Attacking) != 0)
            return false;
    }

    bool __fastcall MaybeAllowPlayerAttacking(Actor* player, void* edx, UInt32 animGroupId)
    {
            //DisablePlayerControlsAlt::g_disabledControls & ~DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Attacking;  
        if ((DisablePlayerControlsAlt::g_disabledControls & DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Attacking) != 0)
            return true;
    }

    //Overheating System
    void WeaponHeat::HeatOnFire()
    {
        float maxHeat = 300.0f;
        float startingHeat = 50.0f;


        heatVal += heatPerShot;

        if (heatVal >= maxHeat)
        {
            isOverheated = true;  
            auto* player = PlayerCharacter::GetSingleton();
            //auto result = ToggleFlag<false>(DisablePlayerControlsAlt::kFlag_Movement);
            //if (result.first) // Only proceed if the flag change was successful
            //{
                DisablePlayerControlsAlt::flags_t changedFlag = DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Sneaking & ~(DisablePlayerControlsAlt::g_disabledControls | static_cast<DisablePlayerControlsAlt::flags_t>(player->disabledControlFlags));; // Disable attacking for everyone    
                if (!changedFlag) 
                    return;

                // Copy code at 0x95F590 to force out of sneak etc 
                if ((changedFlag & DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Movement) != 0) 
                {
                    HUDMainMenu::UpdateVisibilityState(HUDMainMenu::kHUDState_PlayerDisabledControls); 
                }
                else
                {
                    // important for updating RolloverText and such at 0x771972
                    HUDMainMenu::UpdateVisibilityState(HUDMainMenu::kHUDState_RECALCULATE);
                }

                if ((changedFlag & DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Fighting) != 0) 
                    player->SetWantsWeaponOut(0); 

                if ((changedFlag & DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_POV) != 0) 
                {
                    player->bThirdPerson = false; 
                    if (player->playerNode) 
                        player->UpdateCamera(0, 0); 
                    else
                    { 
                        float& g_fThirdPersonZoomHeight = *reinterpret_cast<float*>(0x11E0768); 
                        g_fThirdPersonZoomHeight = 0.0; 
                    }
                }

                if ((changedFlag & DisablePlayerControlsAlt::DisabledControlsFlags::kFlag_Sneaking) != 0) 
                {
                    // force out of sneak by removing sneak movement flag
                    ThisStdCall(0x9EA3E0, PlayerCharacter::GetSingleton()->actorMover, (player->GetMovementFlags() & ~0x400)); 
                }
                AppendToCallChain(0x949CF1, UInt32(MaybePreventPlayerAttacking2), originalAddress); 

            //}
        }
    }
 
    void WeaponHeat::CheckCooldown()
    {
        float startHeatLevel = 50.0f;  // The heat level required to re-enable firing

        // If the weapon is overheated, check the cooldown
        if (isOverheated)
        {
            // If the weapon cools down to or below the starting heat level, re-enable attacking
            if (heatVal <= startHeatLevel)
            {
                isOverheated = false;

                // Re-enable attacking after cooldown
                //auto result = ToggleFlag<true>(DisablePlayerControlsAlt::kFlag_Attacking);  // Re-enable attacking

                //if (result.first)
                //{
                    //DisablePlayerControlsAlt::g_disabledControls &= ~result.second;  // Apply enabling
                //}
            }
        }
    }

    void AllowAttack()
    {
        AppendToCallChain(0x949CF1, UInt32(MaybeAllowPlayerAttacking), originalAddress);
    }
} 