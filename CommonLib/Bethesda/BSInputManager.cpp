#include "BSInputManager.hpp"

void BSInputManager::SetControlHeld(ControlCode aeAction)
{
	ThisStdCall(0xA24280, this, aeAction);
}

void BSInputManager::ResetControlState(ControlCode aeAction)
{
	ThisStdCall(0xA25410, this, aeAction);
}

SInt32 BSInputManager::GetUserAction(ControlCode aeAction, KeyState state)
{
	return ThisStdCall<SInt32>(0xA24660, this, aeAction, state);
}

uint16_t BSInputManager::GetControllerButtonMask(int key) 
{
    int button = 0;
    switch (key)
    {
    case 1:
        button = 1;
        break;
    case 2:
        button = 2;
        break;
    case 4:
        button = 8;
        break;
    case 5:
        button = 4;
        break;
    case 6:
        button = 16;
        break;
    case 7:
        button = 32;
        break;
    case 8:
        button = 64;
        break;
    case 9:
        button = 128;
        break;
    case 10:
        button = 4096;
        break;
    case 11:
        button = 0x2000;
        break;
    case 12:
        button = 0x4000;
        break;
    case 13:
        button = 0x8000;
        break;
    case 14:
        button = 512;
        break;
    case 15:
        button = 256;
        break;
    default:
        return button;
    }
    return button;
}

void BSInputManager::SetControllerAction(ControlCode aeAction, KeyState state)
{
    if (!BSInputManager::GetControllerMode()) return;

    _XINPUT_STATE* currentGamepad = BSInputManager::GetCurrentGamepad();
    _XINPUT_STATE* lastGamepad = BSInputManager::GetLastGamepad();

    UInt8 key = this->ucKeyBinds[Controller][aeAction];
    if (key == 0xFF) return;

    if (key == 16) 
    {
        switch (state) //Left trigger
        {
        case Held:      currentGamepad->Gamepad.bLeftTrigger = 0xFF; break;
        case Pressed:   lastGamepad->Gamepad.bLeftTrigger = 0; currentGamepad->Gamepad.bLeftTrigger = 0xFF; break;
        case Depressed: lastGamepad->Gamepad.bLeftTrigger = 0xFF; currentGamepad->Gamepad.bLeftTrigger = 0; break;
        case Changed:   currentGamepad->Gamepad.bLeftTrigger = (~lastGamepad->Gamepad.bLeftTrigger) & 0xFF; break;
        case None:      lastGamepad->Gamepad.bLeftTrigger = 0; currentGamepad->Gamepad.bLeftTrigger = 0; break;
        default:        break;
        }
    }
    else if (key == 17) 
    {
        switch (state)  //Right trigger
        {
        case Held:      currentGamepad->Gamepad.bRightTrigger = 0xFF; break;
        case Pressed:   lastGamepad->Gamepad.bRightTrigger = 0; currentGamepad->Gamepad.bRightTrigger = 0xFF; break;
        case Depressed: lastGamepad->Gamepad.bRightTrigger = 0xFF; currentGamepad->Gamepad.bRightTrigger = 0; break;
        case Changed:   currentGamepad->Gamepad.bRightTrigger = (~lastGamepad->Gamepad.bRightTrigger) & 0xFF; break;
        case None:      lastGamepad->Gamepad.bRightTrigger = 0; currentGamepad->Gamepad.bRightTrigger = 0; break;
        default:        break;
        }
    }
    else {
        
        uint16_t mask = GetControllerButtonMask(key);
        switch (state) //Face Buttons
        {
        case Held:      currentGamepad->Gamepad.wButtons |= mask; break;
        case Pressed:   lastGamepad->Gamepad.wButtons &= ~mask; currentGamepad->Gamepad.wButtons |= mask; break;
        case Depressed: lastGamepad->Gamepad.wButtons |= mask; currentGamepad->Gamepad.wButtons &= ~mask; break;
        case Changed:   currentGamepad->Gamepad.wButtons = (currentGamepad->Gamepad.wButtons & ~mask) | ((~lastGamepad->Gamepad.wButtons) & mask); break;
        case None:      lastGamepad->Gamepad.wButtons &= ~mask; currentGamepad->Gamepad.wButtons &= ~mask; break;
        default:        break;
        }
    }
}

void BSInputManager::SetUserAction(ControlCode aeAction, KeyState state)
{
    for (int i = 0; i < 4; ++i)
    {
        UInt8 key = this->ucKeyBinds[i][aeAction];
        if (key == 0xFF) continue;

        switch (i)
        {
        case 0: //Keyboard
            switch (state)
            {
            case Held:      this->ucKeyboardState[0][key] |= 0x80; break;
            case Pressed:   this->ucKeyboardState[1][key] = 0; this->ucKeyboardState[0][key] |= 0x80; break;
            case Depressed: this->ucKeyboardState[1][key] |= 0x80; this->ucKeyboardState[0][key] = 0; break;
            case Changed:   this->ucKeyboardState[0][key] = (~this->ucKeyboardState[1][key]) & 0x80; break;
            case None:      this->ucKeyboardState[1][key] = 0; this->ucKeyboardState[0][key] = 0; break;
            default:        break;
            }
            break;

        case 1: //Mouse
            switch (state)
            {
            case Held:      this->kMouseState[0].rgbButtons[key] |= 0x80; break;
            case Pressed:   this->kMouseState[1].rgbButtons[key] = 0; this->kMouseState[0].rgbButtons[key] |= 0x80; break;
            case Depressed: this->kMouseState[1].rgbButtons[key] |= 0x80; this->kMouseState[0].rgbButtons[key] = 0; break;
            case Changed:   this->kMouseState[0].rgbButtons[key] = (~this->kMouseState[1].rgbButtons[key]) & 0x80; break;
            case None:      this->kMouseState[1].rgbButtons[key] = 0; this->kMouseState[0].rgbButtons[key] = 0; break;
            default:        break;
            }
            break;

        case 2: //Controller
            switch (state)
            {
            case Held:      this->kJoyStickState[0][0].rgbButtons[key] |= 0x80; break;
            case Pressed:   this->kJoyStickState[1][0].rgbButtons[key] = 0; this->kJoyStickState[0][0].rgbButtons[key] |= 0x80; break;
            case Depressed: this->kJoyStickState[1][0].rgbButtons[key] |= 0x80; this->kJoyStickState[0][0].rgbButtons[key] = 0; break;
            case Changed:   this->kJoyStickState[0][0].rgbButtons[key] = (~this->kJoyStickState[1][0].rgbButtons[key]) & 0x80; break;
            case None:      this->kJoyStickState[1][0].rgbButtons[key] = 0; this->kJoyStickState[0][0].rgbButtons[key] = 0; break;
            default:        break;
            }
            break;

        case 3: //XInput
            SetControllerAction(aeAction, state);
            break;
        }
    }
}


