#pragma once

#include "Common.h"
#include "Platform.h"

// NOTE: All global game stuff lives here
struct GameContext {
};

void GameInit();
void GameReload();
void GameUpdate();
void GameRender();

// Getters for global variables
// Implemented in GameEntry.cpp
const PlatformState* GetPlatform();
GameContext* GetContext();
const InputState* GetInput();

// Helpers for input handling
inline bool KeyDown(Key key) { return GetInput()->keys[(u32)key].pressedNow; }
inline bool KeyPressed(Key key) { return GetInput()->keys[(u32)key].pressedNow && !GetInput()->keys[(u32)key].wasPressed; }
inline bool MouseButtonDown(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow; }
inline bool MouseButtonPressed(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow && !GetInput()->mouseButtons[(u32)button].wasPressed; }
