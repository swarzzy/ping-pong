#pragma once

#include "SDL.h"

#include <Windows.h>

// For timeBeginPeriod
#include <mmsystem.h>

#define ENABLE_CONSOLE

#include "Win32CodeLoader.h"

#define DISCRETE_GRAPHICS_DEFAULT
#define ENABLE_CONSOLE
#define DEBUG_OPENGL

#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 3

const u32 DefaultWindowWidth = 1280;
const u32 DefaultWindowHeight = 720;

struct OpenGLLoadResult {
    OpenGL* context;
    b32 success;
};

struct Win32Context {
    PlatformState state;

    SDLContext sdl;

    LARGE_INTEGER performanceFrequency;

    LibraryData gameLib;
};

OpenGLLoadResult LoadOpenGL();
f64 GetTimeStamp();
u32 DebugGetFileSize(const wchar_t* filename);
u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const wchar_t* filename);
u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize, const wchar_t* filename);
bool DebugWriteFile(const wchar_t* filename, void* data, u32 dataSize);
FileHandle DebugOpenFile(const wchar_t* filename);
bool DebugCloseFile(FileHandle handle);
u32 DebugWriteToOpenedFile(FileHandle handle, void* data, u32 size);
b32 DebugCopyFile(const wchar_t* source, const wchar_t* dest, bool overwrite);
