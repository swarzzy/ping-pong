#pragma once

#include "RenderQueue.h"

struct Canvas {
    m4x4 projection;
    v4 clearColor;
};

struct Renderer {
    // Should be less than or equal to 2^16 and multiple of 6
    static const u32 MaxBufferCapacity = 1536;

    Canvas canvas;

    GLuint rectColorOpaqueShader;
    GLuint rectColorTransparentShader;
    GLuint lineShader;
    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLint mvpLocation;
    GLint mvpLocationRectColorTransparent;
    GLint mvpLocationLine;

    GLuint multisampledBuffer;
    GLuint multisampledColorTarget;
    GLuint multisampledDepthStencilTarget;

    u32 maxSupportedSampleCount;
    u32 sampleCount;
    uv2 resolution;
};

void RendererInit(Renderer* renderer, uv2 resolution, u32 sampleCount);
void RendererChangeResolution(Renderer* renderer, uv2 newRes, u32 newSampleCount);

void RendererBeginFrame(Renderer* renderer);
void RendererDraw(Renderer* renderer, RenderQueue* queue);
void RendererEndFrame(Renderer* renderer);
