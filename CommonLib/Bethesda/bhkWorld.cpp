#include "bhkWorld.hpp"
#include "AutoMemContext.hpp"
#include "bhkNiCollisionObject.hpp"
#include "bhkUtilFunctions.hpp"
#include "BSXFlags.hpp"
#include "Gamebryo/NiNode.hpp"

bool bhkWorld::SetMotion(NiAVObject* apObject, UInt32 aeType, bool abRecurse, bool abForce, bool abAllowActivate)
{
	return CdeclCall<bool>(0xC6A350, apObject, aeType, abRecurse, abForce, abAllowActivate);
}

bool bhkWorld::EnableCollision(NiAVObject* apObject, bool abEnable, bool abRecurse, bool abForce)
{
	return CdeclCall<bool>(0xC6A0B0, apObject, abEnable, abRecurse, abForce);
}
