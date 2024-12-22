#include "NiObjectNET.hpp"
#include "NiTimeController.hpp"
#include "NiExtraData.hpp"
#include "NiFixedString.hpp"

CRITICAL_SECTION* kExtraDataLock = (CRITICAL_SECTION*)0x11F4380;

NiTimeController* NiObjectNET::GetController(const NiRTTI* apRTTI) const {
    NiTimeController* pControllers = m_spControllers;
    if (!pControllers)
        return nullptr;

    while (pControllers) {
        if (pControllers->IsExactKindOf(apRTTI))
            return pControllers;

        pControllers = pControllers->m_spNext;
    }

    return nullptr;
}

NiTimeController* NiObjectNET::GetController(const NiRTTI& arRTTI) const {
    return GetController(&arRTTI);
}

// 0xA5C480
void NiObjectNET::RemoveController(NiTimeController* apController) {
    ThisStdCall(0xA5C480, this, apController);
}

// 0xA5BDD0
/*NiExtraData* NiObjectNET::GetExtraData(const NiFixedString& arKey) const {
    if (!arKey.m_kHandle)
        return nullptr;

    EnterCriticalSection(kExtraDataLock);
    SInt16 sTop = m_usExtraDataSize - 1;
    SInt16 sBottom = 0;
    SInt16 sMiddle = 0;

    if (sTop < 0) {
        LeaveCriticalSection(kExtraDataLock);
        return nullptr;
    }

    while (sBottom <= sTop) {
        sMiddle = (sTop + sBottom) >> 1;

        ptrdiff_t dtCompare = ((const char*)arKey) - ((const char*)m_ppkExtra[sMiddle]->GetName());
        if (dtCompare == 0) {
            LeaveCriticalSection(kExtraDataLock);
            return m_ppkExtra[sMiddle];
        }
        else if (dtCompare > 0) {
            sBottom = sMiddle + 1;
        }
        else {
            sTop = sMiddle - 1;
        }
    }

    LeaveCriticalSection(kExtraDataLock);
    return nullptr;
}*/

// 0xA5BDD0 Different Definition of GetExtraData just because i guess...
NiExtraData* NiObjectNET::GetExtraData(const NiFixedString& kKey) {
    return ThisStdCall<NiExtraData*>(0xA5BDD0, this, &kKey);
}

// 0xA5BCA0
bool NiObjectNET::AddExtraData(NiExtraData* apExtraData) {
    return ThisStdCall<bool>(0xA5BCA0, this, apExtraData);
}

// 0xA5BC40
bool NiObjectNET::AddExtraData(const NiFixedString& arKey, NiExtraData* apExtraData) {
    return ThisStdCall<bool>(0xA5BC40, this, &arKey, apExtraData);
}

// 0xA5BE90
bool NiObjectNET::RemoveExtraData(const NiFixedString& arKey) {
    return ThisStdCall<bool>(0xA5BE90, this, &arKey);
}

// 0xA5B990
void NiObjectNET::DeleteExtraData(UInt16 ausIndex) {
    ThisStdCall(0xA5B990, this, ausIndex);
}

// 0x4AD1B0
NiObjectNET::CopyType NiObjectNET::GetDefaultCopyType() {
    return *(CopyType*)0x11F4300;
}

// 0x4AD1C0
char NiObjectNET::GetDefaultAppendCharacter() {
    return *(char*)0x11A94A8;
}