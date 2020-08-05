#include "Game.h"
#include "physics/Solver.h"
#include <iostream>

void GameInit() {
    GameContext* context = GetContext();

    Renderer* renderer = &context->renderer;

    RendererInit(&context->renderer);
    RenderQueueInit(&context->renderQueue, 1024);

    renderer->canvas.clearColor = V4(1.0f, 0.4f, 0.0f, 1.0f);
    renderer->canvas.projection = OrthoGLRH(-100.0f, 100.0f, -100.0f, 100.0f, 0.0f, 1.0f);

    Containers::Array<v3> arr1;
    arr1.Insert({-50.0, -50.0, 1.0});
    arr1.Insert({-50.0, -2.0, 1.0});
    arr1.Insert({-40.0, -5.0, 1.0});
    arr1.Insert({-3.0, -45.0, 1.0});
    arr1.Insert({-2.0, -50.0, 1.0});

    Containers::Array<v3> arr2;
    arr2.Insert({50.0, 50.0, 1.0});
    arr2.Insert({50.0, 2.0, 1.0});
    arr2.Insert({2.0, 50.0, 1.0});

    Physics::ConvexBody b1(arr1);
    Physics::ConvexBody b2(arr2);

    b1.SetVelocity({3.5, 0.5});
    b2.SetVelocity({-2.0, -0.1});

    context->physicSolver.AddBody(b1);
    context->physicSolver.AddBody(b2);


#if 0
    // DEMO CODE!!!
    // This code is just demonstration and does not do anything reasonable

    // passing zero as alignment sets it to default
    // standard platform allocator does not requiers any allocator data,
    // so passing nullptr
    context->someData = PlatformAllocate(1024, 0, nullptr);

    context->color1 = V4(0.2f, 0.4f, 0.0f, 1.0f);
    context->color2 = V4(0.1f, 0.7f, 0.0f, 1.0f);
#endif
}

void GameReload() {
}

void GameUpdate() {
    GetContext()->physicSolver.Advance(GetPlatform()->deltaTime);
#if 0
    // This code is just demonstration and does not do anything reasonable
    if (KeyDown(Key::Space)) {
        GameContext* context = GetContext();
        context->color2.b += 0.01f;
        if (context->color2.b > 1.0f) {
            context->color2.b = 0.0f;
        }
    }
#endif
}

void GameRender() {
    GameContext* context = GetContext();
    Renderer* renderer = &context->renderer;
    RenderQueue* queue = &context->renderQueue;

    context->physicSolver.DebugDraw(queue);

    RendererBeginFrame(renderer);
    RendererDraw(renderer, queue);
    RendererEndFrame(renderer);

    RenderQueueReset(queue);

#if 0
    // DEMO CODE!!
    // This code is just demonstration and does not do anything reasonable
    const PlatformState* platform = GetPlatform();

    glViewport(0, 0, platform->windowWidth, platform->windowHeight);

    v4 c1 = context->color1;
    v4 c2 = context->color2;

    if (MouseButtonDown(MouseButton::Left)) {
        glClearColor(c1.r, c1.g, c1.b, c1.a);
    } else {
        glClearColor(c2.r, c2.g, c2.b, c2.a);
    }

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
#endif
}
