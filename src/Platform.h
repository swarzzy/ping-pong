#pragma once

// NOTE: This file is shared by both platform layer and game library
// It defines plaform API. When platform calls game code it passes PlatformState
// structure to the game. This structure contains pointers to all platform functions
// (including OpenGL calls) and other necessary stuff such as current input state.

#include "Common.h"
#include "Intrinsics.h"
#include "Math.h"
#include "platform/OpenGL.h"

#if defined(PLATFORM_WINDOWS)
#define GAME_CODE_ENTRY __declspec(dllexport)
#elif defined(PLATFORM_LINUX)
#define GAME_CODE_ENTRY
#else
#error Unsupported OS
#endif


enum struct GameInvoke : u32
{
    Init, Reload, Update, Render
};

// NOTE: On unix API this should be defined as int
typedef uptr FileHandle;
const FileHandle InvalidFileHandle = Uptr::Max;

typedef u32(DebugGetFileSizeFn)(const wchar_t* filename);
typedef u32(DebugReadFileFn)(void* buffer, u32 bufferSize, const wchar_t* filename);
typedef u32(DebugReadTextFileFn)(void* buffer, u32 bufferSize, const wchar_t* filename);
typedef bool(DebugWriteFileFn)(const wchar_t* filename, void* data, u32 dataSize);
typedef b32(DebugCopyFileFn)(const wchar_t* source, const wchar_t* dest, bool overwrite);

typedef FileHandle(DebugOpenFileFn)(const wchar_t* filename);
typedef bool(DebugCloseFileFn)(FileHandle handle);
typedef u32(DebugWriteToOpenedFileFn)(FileHandle handle, void* data, u32 size);

// NOTE: Functions that platform passes to the game
struct PlatformCalls
{
    DebugGetFileSizeFn* DebugGetFileSize;
    DebugReadFileFn* DebugReadFile;
    DebugReadTextFileFn* DebugReadTextFile;
    DebugWriteFileFn* DebugWriteFile;
    DebugOpenFileFn* DebugOpenFile;
    DebugCloseFileFn* DebugCloseFile;
    DebugCopyFileFn* DebugCopyFile;
    DebugWriteToOpenedFileFn* DebugWriteToOpenedFile;

    // Default allocator
    AllocateFn* Allocate;
    DeallocateFn* Deallocate;
    ReallocateFn* Reallocate;
};

struct KeyState
{
    // TODO: Compress
    b32 pressedNow;
    b32 wasPressed;
};

struct MouseButtonState
{
    // TODO: Compress
    b32 pressedNow;
    b32 wasPressed;
};

enum struct MouseButton : u8
{
    Left = 0, Right, Middle, XButton1, XButton2
};

enum struct Key : u8
{
    Invalid = 0x00,
    // NOTE: currently works only ctrl for both left and right keys
    // right ctrl and super key doesn`t work on linux.
    Ctrl, Space, Apostrophe, Comma,
    Minus, Period, Slash, _0 = 0x30,
    _1, _2, _3, _4, _5, _6, _7, _8,
    _9, Semicolon, Equal, A = 0x41,
    B, C, D, E, F, G, H,
    I, J, K, L, M, N, O,
    P, Q, R, S, T, U, V,
    W, X, Y, Z, LeftBracket,
    BackSlash, RightBracket, Tilde,
    Escape, Enter, Tab, Backspace,
    Insert, Delete, Right, Left,
    Down, Up, PageUp, PageDown,
    Home, End, CapsLock, ScrollLock,
    NumLock, PrintScreen, Pause,
    Return = Enter, F1 = 114,
    F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11,
    F12, F13, F14, F15, F16,
    F17, F18, F19, F20, F21,
    F22, F23, F24, Num0, Num1,
    Num2, Num3, Num4, Num5, Num6,
    Num7, Num8, Num9, NumDecimal,
    NumDivide, NumMultiply, NumSubtract,
    NumAdd, NumEnter = Enter,
    LeftShift = 153, LeftCtrl, Alt,
    LeftSuper, Menu, RightShift,
    RightCtrl, RightSuper, Clear,
    Shift
};

// NOTE: Input state that also gets passed to the game by platform
struct InputState
{
    static const u32 KeyCount = 256;
    static const u32 MouseButtonCount = 5;
    KeyState keys[KeyCount];
    MouseButtonState mouseButtons[MouseButtonCount];
    b32 mouseInWindow;
    b32 activeApp;
    // NOTE: All mouse position values are normalized
    f32 mouseX;
    f32 mouseY;
    f32 mouseFrameOffsetX;
    f32 mouseFrameOffsetY;
    // NOTE: Not normalized
    i32 scrollOffset;
    i32 scrollFrameOffset;
};

struct PlatformState
{
    PlatformCalls functions;
    OpenGL* gl;
    InputState input;
    u64 tickCount;
    i32 fps;
    i32 ups;
    f32 deltaTime;
    u32 windowWidth;
    u32 windowHeight;
};
