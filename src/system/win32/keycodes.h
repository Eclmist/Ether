/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

ETH_NAMESPACE_BEGIN

enum KeyCode
{
    KEYCODE_None            = 0x00, // No key was pressed
    KEYCODE_LButton         = 0x01, // Left mouse button
    KEYCODE_RButton         = 0x02, // Right mouse button
    KEYCODE_Cancel          = 0x03, // Cancel key
    KEYCODE_MButton         = 0x04, // Middle mouse button
    KEYCODE_XButton1        = 0x05, // X1 mouse button
    KEYCODE_XButton2        = 0x06, // X2 mouse button
    // 0x07 is undefined
    KEYCODE_Back            = 0x08,
    KEYCODE_Tab             = 0x09,
    // 0x0A-0B are reserved
    KEYCODE_Clear           = 0x0c, // The CLEAR key
    KEYCODE_Enter           = 0x0d, // The Enter key
    // 0x0E-0F are undefined
    KEYCODE_ShiftKey        = 0x10, // The Shift key
    KEYCODE_ControlKey      = 0x11, // The Ctrl key
    KEYCODE_AltKey          = 0x12, // The Alt key
    KEYCODE_Pause           = 0x13, // The Pause key
    KEYCODE_Capital         = 0x14, // The Caps Lock key
    KEYCODE_CapsLock        = 0x14, // The Caps Lock key
    KEYCODE_KanaMode        = 0x15, // IMI Kana mode
    KEYCODE_HanguelMode     = 0x15, // IMI Hanguel mode (Use HangulMode)
    KEYCODE_HangulMode      = 0x15, // IMI Hangul mode
    // 0x16 is undefined
    KEYCODE_JunjaMode       = 0x17, // IMI Janja mode
    KEYCODE_FinalMode       = 0x18, // IMI Final mode
    KEYCODE_HanjaMode       = 0x19, // IMI Hanja mode
    KEYCODE_KanjiMode       = 0x19, // IMI Kanji mode
    // 0x1A is undefined
    KEYCODE_Escape          = 0x1b, // The ESC key
    KEYCODE_IMEConvert      = 0x1c, // IMI convert key
    KEYCODE_IMINoConvert    = 0x1d, // IMI noconvert key
    KEYCODE_IMEAccept       = 0x1e, // IMI accept key
    KEYCODE_IMIModeChange   = 0x1f, // IMI mode change key
    KEYCODE_Space           = 0x20, // The Space key
    KEYCODE_Prior           = 0x21, // The Page Up key
    KEYCODE_PageUp          = 0x21, // The Page Up key
    KEYCODE_Next            = 0x22, // The Page Down key
    KEYCODE_PageDown        = 0x22, // The Page Down key
    KEYCODE_End             = 0x23, // The End key
    KEYCODE_Home            = 0x24, // The Home key
    KEYCODE_Left            = 0x25, // The Left arrow key
    KEYCODE_Up              = 0x26, // The Up arrow key
    KEYCODE_Right           = 0x27, // The Right arrow key
    KEYCODE_Down            = 0x28, // The Down arrow key
    KEYCODE_Select          = 0x29, // The Select key
    KEYCODE_Print           = 0x2a, // The Print key
    KEYCODE_Execute         = 0x2b, // The Execute key
    KEYCODE_PrintScreen     = 0x2c, // The Print Screen key
    KEYCODE_Snapshot        = 0x2c, // The Print Screen key
    KEYCODE_Insert          = 0x2d, // The Insert key
    KEYCODE_Delete          = 0x2e, // The Delete key
    KEYCODE_Help            = 0x2f, // The Help key
    KEYCODE_D0              = 0x30, // 0
    KEYCODE_D1              = 0x31, // 1
    KEYCODE_D2              = 0x32, // 2
    KEYCODE_D3              = 0x33, // 3
    KEYCODE_D4              = 0x34, // 4
    KEYCODE_D5              = 0x35, // 5
    KEYCODE_D6              = 0x36, // 6
    KEYCODE_D7              = 0x37, // 7
    KEYCODE_D8              = 0x38, // 8
    KEYCODE_D9              = 0x39, // 9
    // 0x3A - 40 are undefined
    KEYCODE_A               = 0x41, // A
    KEYCODE_B               = 0x42, // B
    KEYCODE_C               = 0x43, // C
    KEYCODE_D               = 0x44, // D
    KEYCODE_E               = 0x45, // E
    KEYCODE_F               = 0x46, // F
    KEYCODE_G               = 0x47, // G
    KEYCODE_H               = 0x48, // H
    KEYCODE_I               = 0x49, // I
    KEYCODE_J               = 0x4a, // J
    KEYCODE_K               = 0x4b, // K
    KEYCODE_L               = 0x4c, // L
    KEYCODE_M               = 0x4d, // M
    KEYCODE_N               = 0x4e, // N
    KEYCODE_O               = 0x4f, // O
    KEYCODE_P               = 0x50, // P
    KEYCODE_Q               = 0x51, // Q
    KEYCODE_R               = 0x52, // R
    KEYCODE_S               = 0x53, // S
    KEYCODE_T               = 0x54, // T
    KEYCODE_U               = 0x55, // U
    KEYCODE_V               = 0x56, // V
    KEYCODE_W               = 0x57, // W
    KEYCODE_X               = 0x58, // X
    KEYCODE_Y               = 0x59, // Y
    KEYCODE_Z               = 0x5a, // Z
    KEYCODE_LWin            = 0x5b, // Left Windows key
    KEYCODE_RWin            = 0x5c, // Right Windows key
    KEYCODE_Apps            = 0x5d, // Apps key
    // 0x5E is reserved
    KEYCODE_Sleep           = 0x5f, // The Sleep key
    KEYCODE_NumPad0         = 0x60, // The Numeric keypad 0 key
    KEYCODE_NumPad1         = 0x61, // The Numeric keypad 1 key
    KEYCODE_NumPad2         = 0x62, // The Numeric keypad 2 key
    KEYCODE_NumPad3         = 0x63, // The Numeric keypad 3 key
    KEYCODE_NumPad4         = 0x64, // The Numeric keypad 4 key
    KEYCODE_NumPad5         = 0x65, // The Numeric keypad 5 key
    KEYCODE_NumPad6         = 0x66, // The Numeric keypad 6 key
    KEYCODE_NumPad7         = 0x67, // The Numeric keypad 7 key
    KEYCODE_NumPad8         = 0x68, // The Numeric keypad 8 key
    KEYCODE_NumPad9         = 0x69, // The Numeric keypad 9 key
    KEYCODE_Multiply        = 0x6a, // The Multiply key
    KEYCODE_Add             = 0x6b, // The Add key
    KEYCODE_Separator       = 0x6c, // The Separator key
    KEYCODE_Subtract        = 0x6d, // The Subtract key
    KEYCODE_Decimal         = 0x6e, // The Decimal key
    KEYCODE_Divide          = 0x6f, // The Divide key
    KEYCODE_F1              = 0x70, // The F1 key
    KEYCODE_F2              = 0x71, // The F2 key
    KEYCODE_F3              = 0x72, // The F3 key
    KEYCODE_F4              = 0x73, // The F4 key
    KEYCODE_F5              = 0x74, // The F5 key
    KEYCODE_F6              = 0x75, // The F6 key
    KEYCODE_F7              = 0x76, // The F7 key
    KEYCODE_F8              = 0x77, // The F8 key
    KEYCODE_F9              = 0x78, // The F9 key
    KEYCODE_F10             = 0x79, // The F10 key
    KEYCODE_F11             = 0x7a, // The F11 key
    KEYCODE_F12             = 0x7b, // The F12 key
    KEYCODE_F13             = 0x7c, // The F13 key
    KEYCODE_F14             = 0x7d, // The F14 key
    KEYCODE_F15             = 0x7e, // The F15 key
    KEYCODE_F16             = 0x7f, // The F16 key
    KEYCODE_F17             = 0x80, // The F17 key
    KEYCODE_F18             = 0x81, // The F18 key
    KEYCODE_F19             = 0x82, // The F19 key
    KEYCODE_F20             = 0x83, // The F20 key
    KEYCODE_F21             = 0x84, // The F21 key
    KEYCODE_F22             = 0x85, // The F22 key
    KEYCODE_F23             = 0x86, // The F23 key
    KEYCODE_F24             = 0x87, // The F24 key
    // KEYCODE_// 0x88 - 0x8f are unassigned
    KEYCODE_NumLock         = 0x90, // The Num Lock key
    KEYCODE_Scroll          = 0x91, // The Scroll Lock key
    // KEYCODE_// 0x92 - 96 are OEM specific
    // KEYCODE_// 0x97 - 9f are unassigned
    KEYCODE_LShiftKey       = 0xa0, // The Left Shift key
    KEYCODE_RShiftKey       = 0xa1, // The Right Shift key
    KEYCODE_LControlKey     = 0xa2, // The Left Control key
    KEYCODE_RControlKey     = 0xa3, // The Right Control key
    KEYCODE_LMenu           = 0xa4, // The Left Alt key
    KEYCODE_RMenu           = 0xa5, // The Right Alt key
    KEYCODE_BrowserBack     = 0xa6, // The Browser Back key
    KEYCODE_BrowserForward  = 0xa7, // The Browser Forward key
    KEYCODE_BrowserRefresh  = 0xa8, // The Browser Refresh key
    KEYCODE_BrowserStop     = 0xa9, // The Browser Stop key
    KEYCODE_BrowserSearch   = 0xaa, // The Browser Search key
    KEYCODE_BrowserFavorites= 0xab, // The Browser Favorites key
    KEYCODE_BrowserHome     = 0xac, // The Browser Home key
    KEYCODE_VolumeMute      = 0xad, // The Volume Mute key
    KEYCODE_VolumeDown      = 0xae, // The Volume Down key
    KEYCODE_VolumeUp        = 0xaf, // The Volume Up key
    KEYCODE_MediaNextTrack  = 0xb0, // The Next Track key
    KEYCODE_MediaPreviousTrack  = 0xb1, // The Previous Track key
    KEYCODE_MediaStop       = 0xb2, // The Stop Media key
    KEYCODE_MediaPlayPause  = 0xb3, // The Play/Pause Media key
    KEYCODE_LaunchMail      = 0xb4, // The Start Mail key
    KEYCODE_SelectMedia     = 0xb5, // The Select Media key
    KEYCODE_LaunchApplication1  = 0xb6, // The Launch Application 1 key.
    KEYCODE_LaunchApplication2  = 0xb7, // The Launch Application 2 key.
    // KEYCODE_// 0xB8 - B9 are reserved
    KEYCODE_OemSemicolon    = 0xba, // Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
    KEYCODE_Oem1            = 0xba, // Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
    KEYCODE_OemPlus         = 0xbb, // For any country/region, the '+' key
    KEYCODE_OemComma        = 0xbc, // For any country/region, the ',' key
    KEYCODE_OemMinus        = 0xbd, // For any country/region, the '-' key
    KEYCODE_OemPeriod       = 0xbe, // For any country/region, the '.' key
    KEYCODE_OemQuestion     = 0xbf, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
    KEYCODE_Oem2            = 0xbf, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
    KEYCODE_OemTilde        = 0xc0, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
    KEYCODE_Oem3            = 0xc0, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
    // KEYCODE_// 0xC1 - D7 are reserved
    // KEYCODE_// 0xD8 - DA are unassigned
    KEYCODE_OemOpenBrackets = 0xdb, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
    KEYCODE_Oem4            = 0xdb, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
    KEYCODE_OemPipe         = 0xdc, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
    KEYCODE_Oem5            = 0xdc, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
    KEYCODE_OemCloseBrackets= 0xdd, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
    KEYCODE_Oem6            = 0xdd, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
    KEYCODE_OemQuotes       = 0xde, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
    KEYCODE_Oem7            = 0xde, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
    KEYCODE_Oem8            = 0xdf, // Used for miscellaneous characters; it can vary by keyboard.
    // 0xE0 is reserved
    // 0xE1 is OEM specific
    KEYCODE_OemBackslash    = 0xe2, // Either the angle bracket key or the backslash key on the RT 102-key keyboard
    KEYCODE_Oem102          = 0xe2, // Either the angle bracket key or the backslash key on the RT 102-key keyboard
    // 0xE3 - E4 OEM specific
    KEYCODE_ProcessKey      = 0xe5, // IME Process key
    // 0xE6 is OEM specific
    KEYCODE_Packet          = 0xe7, // Used to pass Unicode characters as if they were keystrokes. The Packet key value is the low word of a 32-bit virtual-key value used for non-keyboard input methods.
    // 0xE8 is unassigned
    // 0xE9 - F5 OEM specific
    KEYCODE_Attn            = 0xf6, // The Attn key
    KEYCODE_CrSel           = 0xf7, // The CrSel key
    KEYCODE_ExSel           = 0xf8, // The ExSel key
    KEYCODE_EraseEof        = 0xf9, // The Erase EOF key
    KEYCODE_Play            = 0xfa, // The Play key
    KEYCODE_Zoom            = 0xfb, // The Zoom key
    KEYCODE_NoName          = 0xfc, // Reserved
    KEYCODE_Pa1             = 0xfd, // The PA1 key
    KEYCODE_OemClear        = 0xfe, // The Clear key
};

ETH_NAMESPACE_END
