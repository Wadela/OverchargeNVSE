#include "NiStream.hpp"

NiObject* NiStream::GetObjectAt(UInt32 uiIndex) {
    return m_kTopObjects.GetAt(uiIndex);
}