#include "NiGlobalStringTable.hpp"

// GAME - 0xA5B690
// GECK - 0x81B0C0
NiGlobalStringTable::GlobalStringHandle NiGlobalStringTable::AddString(const char* pcString) {
    return CdeclCall<GlobalStringHandle>(0xA5B690, pcString);
}

// GAME - 0x43BA60
void NiGlobalStringTable::IncRefCount(GlobalStringHandle& arHandle) {
    if (!arHandle)
        return;

    InterlockedIncrement((size_t*)GetRealBufferStart(arHandle));
}

// GAME - 0x4381D0
void NiGlobalStringTable::DecRefCount(GlobalStringHandle& arHandle) {
    if (!arHandle)
        return;

    InterlockedDecrement((size_t*)GetRealBufferStart(arHandle));
}

UInt32 NiGlobalStringTable::GetLength(const GlobalStringHandle& arHandle) {
    if (!arHandle)
        return 0;

    size_t* pBuffer = reinterpret_cast<size_t*>(GetRealBufferStart(arHandle));
    return static_cast<UInt32>(pBuffer[1]);
}

// GAME - 0x438210
char* NiGlobalStringTable::GetRealBufferStart(const GlobalStringHandle& arHandle) {
    return ((char*)arHandle - 2 * sizeof(size_t));
}