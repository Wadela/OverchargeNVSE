#pragma once

#include "NiGeometryData.hpp"
#include "NiQuaternion.hpp"

NiSmartPointer(NiParticlesData);

class NiParticlesData : public NiGeometryData {
public:
	NiParticlesData();
	virtual ~NiParticlesData();

	virtual void RemoveParticle(UInt16 usParticle);

	bool			bHasRotations;
	UInt16*			pusActiveIndices;
	float*			pfVertexDots;
	float*			m_pfRadii;
	UInt16			m_usActiveVertices;
	float*			m_pfSizes;
	NiQuaternion*	m_pkRotations;
	float*			m_pfRotationAngles;
	NiPoint3*		m_pkRotationAxes;
	UInt8*			m_pucTextureIndices;
	void*			m_pkSubTextureOffsets;
	UInt8			m_ucSubTextureOffsetCount;

	CREATE_OBJECT(NiParticlesData, 0xA96600)
};

//ASSERT_SIZE(NiParticlesData, 0x70);