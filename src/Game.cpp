#include "Game.h"

void GameInit() {
    // This code is just demonstration and does not do anything reasonable
    GameContext* context = GetContext();

    // passing zero as alignment sets it to default
    // standard platform allocator does not requiers any allocator data,
    // so passing nullptr
    context->someData = PlatformAllocate(1024, 0, nullptr);

    context->color1 = V4(0.7f, 0.4f, 0.0f, 1.0f);
    context->color2 = V4(0.4f, 0.7f, 0.0f, 1.0f);
}

void GameReload() {
}

void GameUpdate() {
    // This code is just demonstration and does not do anything reasonable
    if (KeyDown(Key::Space)) {
        GameContext* context = GetContext();
        context->color2.b += 0.01f;
        if (context->color2.b > 1.0f) {
            context->color2.b = 0.0f;
        }
    }
}

void GameRender() {
    // This code is just demonstration and does not do anything reasonable
    const PlatformState* platform = GetPlatform();
    GameContext* context = GetContext();

    glViewport(0, 0, platform->windowWidth, platform->windowHeight);

    v4 c1 = context->color1;
    v4 c2 = context->color2;

    if (MouseButtonDown(MouseButton::Left)) {
        glClearColor(c1.r, c1.g, c1.b, c1.a);
    } else {
        glClearColor(c2.r, c2.g, c2.b, c2.a);
    }

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}
