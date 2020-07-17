#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <Windows.h>

#include "../Common.h"
#include "../Platform.h"

#define ENABLE_CONSOLE

#include "Win32CodeLoader.h"

#define DISCRETE_GRAPHICS_DEFAULT
#define ENABLE_CONSOLE
#define DEBUG_OPENGL

#define OPENGL_MAJOR_VERSION = 4;
#define OPENGL_MINOR_VERSION = 5;

const u32 DefaultWindowWidth = 1280;
const u32 DefaultWindowHeight = 720;

struct Win32Context {
    PlatformState state;

    b32 running;

    SDL_Window* window;
    SDL_Surface* surface;
    SDL_GLContext glContext;

    u8 keyTable[InputState::KeyCount];
};
