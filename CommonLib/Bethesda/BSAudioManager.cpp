#include "BSAudioManager.hpp"
#include "AutoMemContext.hpp"
#include "BSSoundHandle.hpp"
#include "BSWin32Audio.hpp"
#include "INISettingCollection.hpp"

BSAudioManager* BSAudioManager::GetSingleton() {
    return CdeclCall<BSAudioManager*>(0xAD9060);
}

// GAME - 0xADB3E0
// GECK - 0x888E60
bool BSAudioManager::Play(UInt32 auiSoundID, bool abUnk) {
    return ThisStdCall<bool>(0xADB3E0, this, auiSoundID, abUnk);
}

// GAME - 0xADC460
// GECK - 0x889EE0
void BSAudioManager::SetPriority(UInt32 auiSoundID, UInt8 aucPriority) {
    ThisStdCall(0xADC460, this, auiSoundID, aucPriority);
}

// GAME - 0xADC0D0
// GECK - 0x889B50
bool BSAudioManager::SetVolume(UInt32 auiSoundID, float afVolume) {
    return ThisStdCall<bool>(0xADC0D0, this, auiSoundID, afVolume);
}

bool BSAudioManager::SetSpeed(UInt32 auiSoundID, float afSpeed) {
    return ThisStdCall<bool>(0xADB850, this, auiSoundID, afSpeed);
}

void BSAudioManager::FadeInPlay(UInt32 auiSoundID, UInt32 auiMilliseconds) {
    ThisStdCall(0xADC670, this, auiSoundID, auiMilliseconds);
}

void BSAudioManager::FadeTo(UInt32 auiSoundID, UInt32 auiMilliseconds, UInt32 auiFollowUpAction)
{
    ThisStdCall(0xADC560, this, auiSoundID, auiMilliseconds, auiFollowUpAction);
}

// GAME - 0xADB610
// GECK - 0x889090
bool BSAudioManager::Stop(UInt32 auiSoundID) {
    return ThisStdCall<bool>(0xADB610, this, auiSoundID);
}
