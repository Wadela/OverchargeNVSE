#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Real name unknown
class BSInputManager {
private:
	LPCDIDATAFORMAT GetMouseDataFormat();
	LPCDIDATAFORMAT GetKeyboardDataFormat();

public:
	enum Scancodes {
		_Escape = 0x1,
		One = 0x2,
		Two = 0x3,
		Three = 0x4,
		Four = 0x5,
		Five = 0x6,
		Six = 0x7,
		Seven = 0x8,
		Eight = 0x9,
		Nine = 0xA,
		Zero = 0xB,
		Minus = 0xC,
		Equals = 0xD,
		Backspace = 0xE,
		Tab = 0xF,
		Q = 0x10,
		W = 0x11,
		E = 0x12,
		R = 0x13,
		T = 0x14,
		Y = 0x15,
		U = 0x16,
		I = 0x17,
		O = 0x18,
		P = 0x19,
		LeftBracket = 0x1A,
		RightBracket = 0x1B,
		Enter = 0x1C,
		LeftControl = 0x1D,
		A = 0x1E,
		S = 0x1F,
		D = 0x20,
		F = 0x21,
		G = 0x22,
		H = 0x23,
		J = 0x24,
		K = 0x25,
		L = 0x26,
		SemiColon = 0x27,
		Apostrophe = 0x28,
		Tilde = 0x29,
		LeftShift = 0x2A,
		BackSlash = 0x2B,
		Z = 0x2C,
		X = 0x2D,
		C = 0x2E,
		V = 0x2F,
		B = 0x30,
		N = 0x31,
		M = 0x32,
		Comma = 0x33,
		Period = 0x34,
		ForwardSlash = 0x35,
		RightShift = 0x36,
		NumStar = 0x37,
		LeftAlt = 0x38,
		SpaceBar = 0x39,
		CapsLock = 0x3A,
		F1 = 0x3B,
		F2 = 0x3C,
		F3 = 0x3D,
		F4 = 0x3E,
		F5 = 0x3F,
		F6 = 0x40,
		F7 = 0x41,
		F8 = 0x42,
		F9 = 0x43,
		F10 = 0x44,
		NumLock = 0x45,
		ScrollLock = 0x46,
		Num7 = 0x47,
		Num8 = 0x48,
		Num9 = 0x49,
		NumMinus = 0x4A,
		Num4 = 0x4B,
		Num5 = 0x4C,
		Num6 = 0x4D,
		NumPlus = 0x4E,
		Num1 = 0x4F,
		Num2 = 0x50,
		Num3 = 0x51,
		Num0 = 0x52,
		NumPeriod = 0x53,
		F11 = 0x57,
		F12 = 0x58,
		NumEnter = 0x9C,
		RightControl = 0x9D,
		NumSlash = 0xB5,
		PrintScrn = 0xB7,
		RightAlt = 0xB8,
		Home = 0xC7,
		UpArrow = 0xC8,
		PageUp = 0xC9,
		LeftArrow = 0xCB,
		RightArrow = 0xCD,
		End_ = 0xCF,
		DownArrow = 0xD0,
		PageDown = 0xD1,
		Delete_ = 0xD3,
	};

	enum MouseScanCodes {
		LeftMouse = 0x0,
		RightMouse = 0x1,
		MiddleMouse = 0x2,
		MouseButton3 = 0x3,
		MouseButton4 = 0x4,
		MouseButton5 = 0x5,
		MouseButton6 = 0x6,
		MouseButton7 = 0x7,
		MouseWheelUp = 0x8,
		MouseWheelDown = 0x9,
		Mouse_MAX = 0xA,
	};

	enum {
		Keyboard = 0,
		Mouse = 1,
		Controller = 3,
	};

	enum KeyState {
		None = -1,
		Held = 0x0,
		Pressed = 0x1,
		Depressed = 0x2,
		Changed = 0x3,
	};


	enum Flags {
		kFlag_HasJoysticks = 1 << 0,
		kFlag_HasMouse = 1 << 1,
		kFlag_HasKeyboard = 1 << 2,
		kFlag_BackgroundMouse = 1 << 3,
	};

	enum {
		kMaxControlBinds = 0x1C,
	};


	enum {
		kMaxDevices = 8,
	};

	enum ControlCode : __int32
	{
		Forward = 0x0,
		Backward = 0x1,
		Left = 0x2,
		Right = 0x3,
		Attack = 0x4,
		Activate = 0x5,
		Aim = 0x6,
		ReadyItem = 0x7,
		Crouch = 0x8,
		Run = 0x9,
		AlwaysRun = 0xA,
		AutoMove = 0xB,
		Jump = 0xC,
		TogglePOV = 0xD,
		MenuMode = 0xE,
		Rest = 0xF,
		VATS_ = 0x10,
		Hotkey1 = 0x11,
		AmmoSwap = 0x12,
		Hotkey3 = 0x13,
		Hotkey4 = 0x14,
		Hotkey5 = 0x15,
		Hotkey6 = 0x16,
		Hotkey7 = 0x17,
		Hotkey8 = 0x18,
		QuickSave = 0x19,
		QuickLoad = 0x1A,
		Grab = 0x1B,
		Escape = 0x1C,
		Console = 0x1D,
		Screenshot = 0x1E,
	};

	struct JoystickObjectsInfo {
		enum {
			kHasXAxis = 1 << 0,
			kHasYAxis = 1 << 1,
			kHasZAxis = 1 << 2,
			kHasXRotAxis = 1 << 3,
			kHasYRotAxis = 1 << 4,
			kHasZRotAxis = 1 << 5
		};

		UInt32	axis;
		UInt32	buttons;
	};

	struct VibrationStates {
		UInt32	uiMotorStrength;
		UInt32	uiPattern;
		UInt32	uiDurationMS;
		UInt32	uiPulseDuration;
		UInt8	byte10;
		UInt32	unk14;
		UInt16	wrd18[2];
		UInt32	unk1C;
		UInt16	wrd20[2];
	};


	struct ControllerVibration {
		struct MotorParams {
			UInt32 duration;
			UInt32 unk04;
			UInt32 unk08;
			UInt32 uiTimePassed;
			UInt32 unk10;
		};


		UInt32				uiControllerIndex;
		VibrationStates*	pLeftStates;
		VibrationStates*	pRightStates;
		MotorParams			LeftParams;
		MotorParams			RightParams;
		UInt32				unk34;
		UInt32				uiTickCount;
		UInt32				unk3C;
	};

	struct MouseData {
		SInt32 xDelta;
		SInt32 yDelta;
		SInt32 iMouseWheelScroll;
		bool   bCurrButtonStates[8];
	};

	enum _XINPUT_BUTTONS : uint16_t {
		XINPUT_GAMEPAD_DPAD_UP			= 0x0001,
		XINPUT_GAMEPAD_DPAD_DOWN		= 0x0002,
		XINPUT_GAMEPAD_DPAD_LEFT		= 0x0004,
		XINPUT_GAMEPAD_DPAD_RIGHT		= 0x0008,
		XINPUT_GAMEPAD_START			= 0x0010,
		XINPUT_GAMEPAD_BACK				= 0x0020,
		XINPUT_GAMEPAD_LEFT_THUMB		= 0x0040,
		XINPUT_GAMEPAD_RIGHT_THUMB		= 0x0080,
		XINPUT_GAMEPAD_LEFT_SHOULDER	= 0x0100,
		XINPUT_GAMEPAD_RIGHT_SHOULDER	= 0x0200,
		XINPUT_GAMEPAD_A				= 0x1000,
		XINPUT_GAMEPAD_B				= 0x2000,
		XINPUT_GAMEPAD_X				= 0x4000,
		XINPUT_GAMEPAD_Y				= 0x8000,
	};

	struct _XINPUT_GAMEPAD {
		uint16_t wButtons;
		uint8_t bLeftTrigger;
		uint8_t bRightTrigger;
		uint16_t sThumbLX;
		uint16_t sThumbLY;
		uint16_t sThumbRX;
		uint16_t sThumbRY;
	};

	struct _XINPUT_STATE {
		uint32_t dwPacketNumber;
		_XINPUT_GAMEPAD Gamepad;
	};

	bool bIsControllerDisabled;
	UInt8 byte0001;
	UInt8 byte0002;
	UInt8 byte0003;
	Bitfield32 uiFlags;
	LPDIRECTINPUT8A pDirectInput;
	LPDIRECTINPUTDEVICE8A pDevices[8];
	LPDIRECTINPUTDEVICE8A pKeyboard;
	LPDIRECTINPUTDEVICE8A pMouse;
	DIJOYSTATE kJoyStickState[8][2];
	DIDEVICEINSTANCEA kJoyStickInstances[8];
	DIDEVCAPS kJoyStickCaps[8];
	UInt32 uiJoyStickFlags[8][2];
	UInt32 uiJoyStickCount;
	UInt8 ucKeyboardState[2][256];
	DIDEVCAPS mouseCaps;
	DIMOUSESTATE2 kMouseState[2];
	BOOL bSwapLeftRightMouseButtons;
	UInt8 cMouseSensitivity;
	UInt8 byte1B51;
	UInt8 byte1B52;
	UInt8 byte1B53;
	UInt32 uiDoubleClickTime;
	bool bDoubleClicked[8];
	UInt32 uiLastClickedTime[8];
	ControllerVibration* pControllerVibration;
	VibrationStates* pVibrationProperties;
	UInt8 isControllerEnabled;
	UInt8 byte1B89;
	UInt8 byte1B8A;
	UInt8 byte1B8B;
	UInt32 eActionToSet;
	bool bSettingAControl;
	bool bUpdated;
	UInt16 usOverrideFlags;
	UInt8 ucKeyBinds[4][28];

	__forceinline static BSInputManager* GetSingleton() { return *reinterpret_cast<BSInputManager**>(0x011F35CC); };
	__forceinline static _XINPUT_STATE* GetCurrentGamepad() { return reinterpret_cast<_XINPUT_STATE*>(0x011F35A8); };
	__forceinline static _XINPUT_STATE* GetLastGamepad() { return reinterpret_cast<_XINPUT_STATE*>(0x011F35B8); };
	__forceinline static bool GetControllerMode() { return *reinterpret_cast<bool*>(0x011F35C8); };

	static LPDIRECTINPUT8A GetDirectInput();
	static LPDIRECTINPUTDEVICE8A GetKeyboard();
	static LPDIRECTINPUTDEVICE8A GetMouse();

	void ToggleMouse(bool abDisable);
	void ToggleKeyboard(bool abDisable);
	void ChangeMouseCooperativeLevel(bool abBackground);
	bool GetButtonState(UInt32 uiInputDevice, UInt8 aucKey, UInt32 auiState);
	SInt32 GetControlState(UInt8 aucKey, UInt32 auiState);
	uint16_t GetControllerButtonMask(int key);

	void   SetControlHeld(ControlCode aeAction);
	void   ResetControlState(ControlCode aeAction);

	SInt32 GetUserAction(ControlCode aeAction, KeyState state);
	void   SetControllerAction(ControlCode aeAction, KeyState state);
	void   SetUserAction(ControlCode aeAction, KeyState state);
};

ASSERT_SIZE(BSInputManager, 0x1C04);