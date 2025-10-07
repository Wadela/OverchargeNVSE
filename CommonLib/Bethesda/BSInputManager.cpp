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

void BSInputManager::SetUserAction(ControlCode aeAction, KeyState state)
{
    for (int i = 0; i < 3; ++i)
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
        }
        //case 3: xinput?
    }
}


