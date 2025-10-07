#pragma once

#include "NiRefObject.hpp"
#include "BSString.hpp"

class BSFaceGenMorphDataHair;
NiSmartPointer(BSFaceGenMorphData);
NiSmartPointer(NiGeometry);
NiSmartPointer(BSFaceGenModel);

class BSFaceGenModel : public NiRefObject {
public:
	class String : public BSString {
	public:
		DWORD dword8;
	};

	class MeshData {
	public:
		BSString				strEGMPath;
		UInt32					unk008;
		NiRefObjectPtr			spObject0C;
		NiGeometryPtr			spGeometry10;
		UInt32					unk014;
		UInt32					unk018;
		BSFaceGenMorphDataPtr	spMorphData;
		UInt32					uiRefCount;
	};

	MeshData*	pMeshData;
	String*		pString;
};