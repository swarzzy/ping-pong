#pragma once

#include "RenderQueue.h"

struct Canvas {
    m4x4 projection;
    v4 clearColor;
};

enum struct TextureFilter : u32 {
    None = 0, Bilinear, Trilinear, Anisotropic, Default = Bilinear
};

enum struct TextureFormat : u32 {
    Unknown = 0, SRGBA8, SRGB8, RGBA8, RGB8, RGB16F, RG16F, R8, RG8, RG32F,
};

enum struct TextureWrapMode : u32 {
    Repeat = 0, ClampToEdge, Default = Repeat
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

u32 RendererLoadTexture(Renderer* renderer, u32 width, u32 height, TextureFormat format, void* pixels = nullptr, TextureFilter filter = TextureFilter::Default, TextureWrapMode wrapMode = TextureWrapMode::Default);
