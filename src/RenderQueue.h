#pragma once

#include "Common.h"

enum struct RenderCommandType : u32 {
    RectColor,
};

struct RenderCommand {
    RenderCommandType type;

    b32 transparent;

    union {
        struct {
            v2 min;
            v2 max;
            v4 color;
            f32 z;
        } rectColor;
    };
};

struct RenderQueue {
    u32 size;
    u32 at;
    RenderCommand* buffer;
};

void RenderQueueInit(RenderQueue* queue, u32 size);
void RenderQueuePush(RenderQueue* queue, RenderCommand command);
void RenderQueueReset(RenderQueue* queue);
