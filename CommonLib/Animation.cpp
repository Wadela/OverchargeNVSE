#include "Animation.hpp"
#include "BSAnimGroupSequence.hpp"

void Animation::SetCurrentSequence(BSAnimGroupSequence* destAnim, bool resetSequenceState)
{
	if (!destAnim || !destAnim->spAnimGroup)
		return;

	const auto sequenceId = destAnim->spAnimGroup->GetGroupInfo()->ucSequenceType;
	const auto animGroupId = destAnim->spAnimGroup->usGroupID;

	constexpr UInt8 kMaxSequences = 8;
	if (sequenceId >= kMaxSequences)
		return;

	pAnimSequence[sequenceId] = destAnim;
	usGroupIDs[sequenceId] = TESAnimGroup::Type(animGroupId);

	if (resetSequenceState)
		uiActions[sequenceId] = 0;
}