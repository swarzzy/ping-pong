#include "Game.h"

void GameInit() {
}

void GameReload() {
}

void GameUpdate() {
}

void GameRender() {
    auto platform = GetPlatform();
    glViewport(0, 0, platform->windowWidth, platform->windowHeight);
    glClearColor(0.7f, 0.3f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}
