#pragma once
#include "MainHeader.hpp"
#include "WeaponHeat.hpp"
#include "TESForm.hpp"

namespace Overcharge
{
    extern std::unordered_map<UInt32, HeatInfo> heatDataMap;

    void InitWeaponData()
    {
        UInt32 laerID = TESForm::GetFormIDByEdID("NVDLC03WeapLaer");
        UInt32 plasmaRifleID = TESForm::GetFormIDByEdID("WeapPlasmaRifle");

        heatDataMap = {
            {laerID, HeatInfo(0.0f, 40.0f, 30.0f, 300.0f)},
            {plasmaRifleID, HeatInfo(0.0f, 40.0f, 30.0f, 300.0f)}
        };
    }
}