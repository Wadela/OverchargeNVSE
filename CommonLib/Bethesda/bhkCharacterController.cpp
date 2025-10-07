#include "bhkCharacterController.hpp"

// GAME - 0xC6E390
void bhkCharacterController::SetPosition(const hkVector4& arPosition) {
	ThisStdCall(0xC6E390, this, &arPosition);
}
