#include "PlayerCharacter.hpp"

NiNode* PlayerCharacter::GetCamera1st() {
	return *reinterpret_cast<NiNode**>(0x11E07D0);
}

NiNode* PlayerCharacter::GetCamera3rd() {
	return *reinterpret_cast<NiNode**>(0x11E07D4);
}

// GAME - 0x950BB0
NiNode* PlayerCharacter::GetNode(bool abFirstPerson) const {
    return ThisStdCall<NiNode*>(0x950BB0, this, abFirstPerson);
}
