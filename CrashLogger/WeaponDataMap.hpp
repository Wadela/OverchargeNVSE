#pragma once
#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "TESForm.hpp"

namespace Overcharge
{
    extern std::unordered_map<UInt32, HeatData> weaponDataMap;

    void InitWeaponData()
    {
        UInt32 laerID = TESForm::GetFormIDByEdID("NVDLC03WeapLaer");
        UInt32 plasmaRifleID = TESForm::GetFormIDByEdID("WeapPlasmaRifle");

        weaponDataMap =
        {
            {laerID, HeatData(HeatInfo(0.0f, 30.0f, 20.0f, 300.0f), HeatFX(NiColor(1, 0, 0), NiColor(0, 1, 0), {"##PLRPlane1:0", "TestString"}))},
            {plasmaRifleID, HeatData(HeatInfo(0.0f, 40.0f, 20.0f, 300.0f), HeatFX(NiColor(0.655f, 1.000f, 0.486f), NiColor(0.486f, 0.900f, 1.000f), {"##PLRPlane1:0", "##PLRCylinder1:0", "##PLRCylinder2:0", "##PLRCylinder3:0"}))}
        };
    }
}