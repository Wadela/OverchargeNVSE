#include "TESAnimGroup.hpp"
#include <span>

TESAnimGroup::AnimGroupInfo* GetGroupInfo(TESAnimGroup::Type groupId)
{
	return reinterpret_cast<TESAnimGroup::AnimGroupInfo*>(0x11977D8) + groupId;
}

TESAnimGroup::AnimGroupInfo* TESAnimGroup::GetGroupInfo() const
{
	return ::GetGroupInfo(GetBaseGroupID());
}

TESAnimGroup::Type TESAnimGroup::GetBaseGroupID() const
{
	return static_cast<TESAnimGroup::Type>(usGroupID);
}
