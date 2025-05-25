#pragma once

#include "NiKeyBasedInterpolator.hpp"
#include "NiQuatTransform.hpp"

NiSmartPointer(NiTransformData);

//Credit to JIP LN NVSE for struct
enum InterpKeyType
{
	kKeyType_Linear = 1,
	kKeyType_Quadratic,
	kKeyType_TBC,
	kKeyType_XYZ,
	kKeyType_Const
};

template <typename T_Data> struct InterpKey
{
	float			time;
	T_Data			value;
};

class NiTransformData : public NiObject
{
public:
	UInt16						numRotationKeys;	// 08
	UInt16						numTranslationKeys;	// 0A
	UInt16						numScaleKeys;		// 0C
	UInt16						pad0E;				// 0E
	UInt32						rotationKeyType;	// 10
	UInt32						translationKeyType;	// 14
	UInt32						scaleKeyType;		// 18
	UInt8						rotationKeySize;	// 1C
	UInt8						translationKeySize;	// 1D
	UInt8						scaleKeySize;		// 1E
	UInt8						pad1F;				// 1F
	InterpKey<NiQuaternion>*	rotationKeys;		// 20
	InterpKey<NiPoint3>*		translationKeys;	// 24
	InterpKey<float>*			scaleKeys;			// 28
};
ASSERT_SIZE(NiTransformData, 0x2C);