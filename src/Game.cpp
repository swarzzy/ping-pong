#include "Game.h"

void GameInit() {
    GameContext* context = GetContext();
    const PlatformState* platform = GetPlatform();

    Renderer* renderer = &context->renderer;

    RendererInit(&context->renderer, UV2(platform->windowWidth, platform->windowHeight), 4);

    RenderQueueInit(&context->renderQueue, 1024);

    renderer->canvas.clearColor = V4(1.0f, 0.4f, 0.0f, 1.0f);
    renderer->canvas.projection = OrthoGLRH(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 1.0f);
}

void GameReload() {
}

void GameUpdate() {
    const PlatformState* platform = GetPlatform();
    GameContext* context = GetContext();

    if (platform->windowWidth != context->renderer.resolution.x || platform->windowHeight != context->renderer.resolution.y) {
        RendererChangeResolution(&context->renderer, UV2(platform->windowWidth, platform->windowHeight), context->renderer.sampleCount);
    }
}

void GameRender() {
    GameContext* context = GetContext();
    Renderer* renderer = &context->renderer;
    RenderQueue* queue = &context->renderQueue;

    DrawQuad(queue, V2(1.5f), V2(3.0f), 0.1f, V4(1.0f, 1.0f, 1.0f, 1.0f));
    DrawQuad(queue, V2(0.0f), V2(2.0f), 0.2f, V4(1.0f, 1.0f, 0.0f, 1.0f));
    DrawQuad(queue, V2(2.0f), V2(4.0f), 1.0f, V4(0.0f, 0.0f, 1.0f, 1.0f));

    //m4x4 transform = Translate(V3(-3.0f, -5.0f, 0.0f)) * Scale(V3(2.3f));// * M4x4(3.0f);//Rotate(0.0f, 45.0f, 0.0f);
    static f32 time = 0.0f;
    time += GetPlatform()->deltaTime * 20.0f;
    m4x4 transform = Translate(V3(0.0f, 0.0f, 1.0f)) * Rotate(0.0f, 0.0f, time);

    DrawQuad(queue, V4(1.0f, 0.0f, 0.0f, 1.0f), &transform, V2(0.7f));

    //void DrawQuad(RenderQueue* queue, v3 min, v3 max, v4 color, const m4x4* transform);
    //void DrawQuad(RenderQueue* queue, v4 color, const m4x4* transform, v2 anchor = {});


    RendererBeginFrame(renderer);
    RendererDraw(renderer, queue);
    RendererEndFrame(renderer);

    RenderQueueReset(queue);
}
