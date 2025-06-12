#pragma once

#include "NiPSysEmitter.hpp"
#include "NiAVObject.hpp"

NiSmartPointer(NiPSysVolumeEmitter);

class NiPSysVolumeEmitter : public NiPSysEmitter {
public:
	NiPSysVolumeEmitter();
	virtual ~NiPSysVolumeEmitter();

	NiAVObject* m_pkEmitterObj;

	NiAVObject* SetEmitterObj(NiAVObject* emitter) { return ThisStdCall<NiAVObject*>(0xC1FA30, this); }
};

ASSERT_SIZE(NiPSysVolumeEmitter, 0x58)