#pragma once

#include "DList.hpp"
#include "NiTArray.hpp"
#include "NiNode.hpp"
#include "NiTList.hpp"

NiSmartPointer(BSMasterParticleSystem);

class BSMasterParticleSystem : public NiNode {
public:
	BSMasterParticleSystem();
	virtual ~BSMasterParticleSystem();

	NiTList<NiAVObjectPtr>			kEmitterObjList;
	UInt16							usActiveEmitterObjCount;
	UInt16							usMaxEmitterObj;
	UInt16							wordBC;
	NiAVObjectPtr*					kEmitterIterator;
	UInt32							uiIndex;
	UInt32							uiNodeIndex;
	NiTPrimitiveArray<NiParticles*> kChildParticles;
	float							fMasterPSysTime;

	CREATE_OBJECT(BSMasterParticleSystem, 0xC5E5A0)
};

ASSERT_SIZE(BSMasterParticleSystem, 0xE0)