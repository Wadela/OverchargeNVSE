#include "ShellNVSE.h"

namespace PluginFunctions
{
    //Extern Functions in ppNVSE.h
    AuxVector* (*GetMemberVar)(const TESForm* form, const char* varName, const TESForm* linkedObj, const char* slot, UInt8 priority);
    bool pNVSE = false;

    void init_pNVSE()
    {
        HMODULE m_pNVSE = GetModuleHandle("pNVSE.dll");
        if (m_pNVSE != NULL) {

            GetMemberVar = (AuxVector * (*)(const TESForm * form, const char* varName, const TESForm * linkedObj, const char* slot, UInt8 priority))GetProcAddress(m_pNVSE, MAKEINTRESOURCEA(4));
            if (GetMemberVar == NULL) {
                gLog.Message("One or more function pointers from pNVSE are invalid");
            }
            else {
                gLog.Message("All pNVSE Functions Valid");
                pNVSE = true;
            }
        }
        else {
            gLog.Message("pNVSE not installed");
        }
    }
}