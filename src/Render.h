#pragma once

#include "RenderQueue.h"

struct Canvas {
    v3 min;
    v3 max;
    v4 clearColor;
};

struct Renderer {
    // Should be less than or equal to 2^16 and multiple of 4
    static const u32 MaxBufferCapacity = 1024;

    Canvas canvas;

    GLuint rectColorOpaqueShader;
    GLuint vertexBuffer;
    GLuint indexBuffer;
};

void RendererInit(Renderer* renderer);
