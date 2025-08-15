#pragma once

#include "BSShaderProperty.hpp"
#include "NiAVObject.hpp"
#include "NiColor.hpp"
#include "NiPoint4.hpp"

class ParticleShaderProperty : public BSShaderProperty {
public:
	struct InstanceData {
		NiPoint3	kUnk00;
		float		fUnk0C;
		NiPoint3	kUnk10;
		NiPoint3	kUnk1C;
		float		fUnk28;
		float		fUnk2C;
	};

	InstanceData* pParticleData;
	uint32_t								unk64;
	float									unk068;
	bool									byte06C;
	uint32_t								unk070;
	float									unk074;
	float									unk078;
	float									unk07C;
	float									fLifetimeVar;
	float									unk084;
	float									unk088;
	float									unk08C;
	NiPoint3								kVelocity;
	NiPoint3								kAcceleration;
	float									fInitRotDeg;
	float									fInitRotDegVar;
	float									fRotSpeedDegPerSec;
	float									fParticleRotSpeedDegPerSecVar;
	float									fColorKey1Time;
	float									fColorKey2Time;
	float									fColorKey3Time;
	NiColorA								kColor1;
	NiColorA								kColor2;
	NiColorA								kColor3;
	NiPoint2								kScaleKeys;
	NiPoint2								kScaleKeyTimes;
	float									fTime_104;
	uint32_t								eSourceBlendMode;
	uint32_t								eDestBlendMode;
	uint32_t								eBlendOp;
	uint32_t								eZTestFunc;
	NiPointer<NiTexture>					spTexture_118;
	NiTObjectArray<NiPointer<NiAVObject>>	unk11C;
	NiNode*									node12C;
	float									fFade;
	float									fTextureCountU;
	float									fTextureCountV;
	float									unk13C;
	float									unk140;
	float									unk144;
	float									fExplosionWindSpeed;
};
ASSERT_SIZE(ParticleShaderProperty, 0x14C);