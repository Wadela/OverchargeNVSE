#pragma once

#include "BSAudio.hpp"
#include <dsound.h>

class BSWin32Audio : public BSAudio {
public:
	BSWin32Audio();
	virtual ~BSWin32Audio();
	
	struct BufferCaps : public DSBCAPS {
		UInt32					unk054;
		UInt32					unk058;
		UInt32					unk05C;
		BOOL					bUseHRTF;
		UInt32					unk064;
		UInt32					unk068;
		UInt32					unk06C;
		UInt32					unk070;
		UInt32					unk074;
		UInt32					unk078;
		UInt32					unk07C;
		UInt32					unk080;
		UInt32					unk084;
		UInt32					unk088;
		UInt32					unk08C;
		UInt32					unk090;
		UInt32					unk094;
		UInt32					unk098;
		UInt32					unk09C;
	};


	LPDIRECTSOUND8			pDirectSound8;
	LPDIRECTSOUNDBUFFER8	pDirectSoundBuffer;
	BufferCaps				dsbCaps;
	HWND					hWindow;

	static BSWin32Audio* GetSingleton() { return *(BSWin32Audio**)0x11F6D98; };
	BSSoundHandle GetSoundHandleByFilePath(const char* apFileName, uint32_t aeAudioFlags, TESSound* apSound) {
		BSSoundHandle kHandle;
		ThisCall(0xAD7480, this, &kHandle, apFileName, aeAudioFlags, apSound);
		return kHandle;
	}

	BSSoundHandle GetSoundHandleByFormID(UInt32 auiFormID, uint32_t aeAudioFlags) {
		BSSoundHandle kHandle;
		ThisCall(0xAD73B0, this, &kHandle, auiFormID, aeAudioFlags);
		return kHandle;
	}

	BSSoundHandle GetSoundHandleByEditorName(const char* apEditorID, uint32_t aeAudioFlags) {
		BSSoundHandle kHandle;
		ThisCall(0xAD7550, this, &kHandle, apEditorID, aeAudioFlags);
		return kHandle;
	}

	static void __stdcall Create();

	void ControlAudioThread(bool abEnable);
};
ASSERT_SIZE(BSWin32Audio, 0xA4);
ASSERT_SIZE(BSWin32Audio::BufferCaps, 0x60);