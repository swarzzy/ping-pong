#pragma once

#include "Common.h"

enum struct RenderCommandType : u32 {
    RectColor, Line
};

struct RenderCommand {
    RenderCommandType type;

    b32 transparent;

    // TODO(swarzzy): If we are going to do something with this queue
    // like, for example, sorting, then we probably should store command itself
    // and it's data separately
    union {
        struct {
            m4x4 transform;
            v4 color;
            v2 min;
            v2 max;
            f32 z;
        } rectColor;
        struct {
            v3 begin;
            v3 end;
            v4 color;
            // Not supported for now
            f32 thickness;
        } line;
    };
};

struct RenderQueue {
    u32 rectBufferSize;
    u32 rectBufferAt;
    u32 lineBufferSize;
    u32 lineBufferAt;
    RenderCommand* rectBuffer;
    // TODO(swarzzy): Should we use separate buffers for different command or just
    // use one and sort it?
    RenderCommand* lineBuffer;
};

void RenderQueueInit(RenderQueue* queue, u32 size);
void RenderQueuePush(RenderQueue* queue, RenderCommand command);
void RenderQueueReset(RenderQueue* queue);

// Helpers
void DrawQuad(RenderQueue* queue, v2 min, v2 max, f32 z, v4 color);
// TODO(swarzzy): Make shure this functions are inlined
void DrawQuad(RenderQueue* queue, v3 min, v3 max, v4 color, const m4x4* transform);

// Anchor defines placement of quad's local basis origin in barycentric coords.
// For example, if you set anchor to {0.5f, 0.5f} then the origin of a quad will be in it's center,
// otherwise if you place it at {1.0f, 1.0f} then the origin will be placed at upper right corner of the a quad
void DrawQuad(RenderQueue* queue, v4 color, const m4x4* transform, v2 anchor = {});
void DrawLine(RenderQueue* queue, v3 begin, v3 end, v4 color);
