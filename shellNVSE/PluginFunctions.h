#pragma once

namespace PluginFunctions 
{
    void init_pNVSE();
    extern bool pNVSE;
    extern AuxVector* (*GetMemberVar)(const TESForm* form, const char* varName, const TESForm* linkedObj, const char* slot, UInt8 priority);
}