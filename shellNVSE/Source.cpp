#include "MainHeader.h"
#include "InitHooks.h"
#include "NiNodes.h"
#include "NiTypes.h"
#include "NiPoint.h"
#include "NiObjects.h"
#include "GameObjects.h"



void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName)
{
	actorRef = *g_thePlayer; 
	if (NiNode* niNode = actorRef->GetNiNodeJIP())   
	{
		if (NiAVObject* block = niNode->GetBlock(blockName))
		{
			if (NiMaterialProperty* matProp = ((NiGeometry*)block)->materialProp)
			{
				matProp->emissiveRGB.r = 0;
				matProp->emissiveRGB.g = 0;
				matProp->emissiveRGB.b = 0;
			}
		}
	}
}


