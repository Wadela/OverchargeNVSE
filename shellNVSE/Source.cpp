#include "MainHeader.h"
#include "InitHooks.h"
#include "NiNodes.h"
#include "NiTypes.h"
#include "NiPoint.h"
#include "NiObjects.h"
#include "GameObjects.h"


namespace Overcharge
{

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

	char __cdecl TestFunc(ExtraDataList* a2)
	{
		Console_Print("Blah Blah Break...");
		
		return 0;
	}

	void InitHooks()
	{
		WriteRelCall(0x5AC763, (UInt32) TestFunc);   
		//0x5AC750 - looks like Actor::OnFire
		//0x523150 - from jip under DoFireWeaponEX
		//0x9BAD76, 0x1087F1C - from jip under CMD_FireWeaponEX_Execure
		//0x400000 - Enum Script Locals
	}

}
