#pragma once

#include "NiPoint3.hpp"

class BSSoundHandle {
public:
	UInt32	uiSoundID;
	bool	bAssumeSuccess;
	UInt32	uiState;

	BSSoundHandle() : uiSoundID(-1), bAssumeSuccess(false), uiState(0) {}
	~BSSoundHandle() {}

	BSSoundHandle operator=(const BSSoundHandle& arHandle) {
		uiSoundID = arHandle.uiSoundID;
		bAssumeSuccess = arHandle.bAssumeSuccess;
		uiState = arHandle.uiState;
		return *this;
	}

	BSSoundHandle operator=(const BSSoundHandle* apHandle) {
		uiSoundID = apHandle->uiSoundID;
		bAssumeSuccess = apHandle->bAssumeSuccess;
		uiState = apHandle->uiState;
		return *this;
	}

	bool IsValid();
	bool IsInvalid() const { return uiSoundID == -1; }

	bool IsPlaying() const {
		return ThisStdCall<bool>(0xAD8930, this);
	}

	bool Play(bool abUnk) {
		return ThisStdCall<bool>(0xAD8830, this, abUnk);
	}

	bool FadeInPlay(uint32_t auiMilliseconds) {
		return ThisStdCall<bool>(0xAD8D60, this, auiMilliseconds);
	}

	bool Stop() {
		return ThisStdCall<bool>(0xAD88F0, this);
	}

	bool FadeOutAndRelease(uint32_t auiMilliseconds) {
		return ThisStdCall<bool>(0xAD8DA0, this, auiMilliseconds);
	}

	bool SetPosition(NiPoint3 akPosition) {
		return ThisStdCall<bool>(0xAD8B60, this, akPosition);
	}

	void SetObjectToFollow(NiAVObject* apObject) {
		ThisStdCall(0xAD8F20, this, apObject);
	}

	bool SetVolume(float afVolume) const {
		return ThisStdCall<bool>(0xAD89E0, this, afVolume);
	}

};

ASSERT_SIZE(BSSoundHandle, 0xC);