#include "RenderQueue.h"

void RenderQueueInit(RenderQueue* queue, u32 size) {
    queue->rectBufferSize = size;
    queue->rectBufferAt = 0;
    queue->lineBufferSize = size;
    queue->lineBufferAt = 0;
    queue->rectBuffer = (RenderCommand*)PlatformAllocate(sizeof(RenderCommand) * size, 0, nullptr);
    queue->lineBuffer = (RenderCommand*)PlatformAllocate(sizeof(RenderCommand) * size, 0, nullptr);
    assert(queue->rectBuffer);
    assert(queue->lineBuffer);
}

void RenderQueuePush(RenderQueue* queue, RenderCommand command) {
    switch (command.type) {
    case RenderCommandType::RectColor: {
        if (queue->rectBufferAt < queue->rectBufferSize) {
            queue->rectBuffer[queue->rectBufferAt] = command;
            queue->rectBufferAt++;
        }
    } break;
    case RenderCommandType::Line: {
        if (queue->lineBufferAt < queue->lineBufferSize) {
            queue->lineBuffer[queue->lineBufferAt] = command;
            queue->lineBufferAt++;
        }
    } break;
    invalid_default();
    }
}

void RenderQueueReset(RenderQueue* queue) {
    queue->rectBufferAt = 0;
    queue->lineBufferAt = 0;
}

void DrawQuad(RenderQueue* queue, v2 min, v2 max, f32 z, v4 color) {
    RenderCommand command {};
    command.type = RenderCommandType::RectColor;
    command.rectColor.min = min;
    command.rectColor.max = max;
    command.rectColor.z = z;
    command.rectColor.color = color;
    command.rectColor.transform = M4x4(1.0f);

    RenderQueuePush(queue, command);
}

void DrawQuad(RenderQueue* queue, v2 min, v2 max, f32 z, v4 color, const m4x4* transform) {
    RenderCommand command {};
    command.type = RenderCommandType::RectColor;
    command.rectColor.min = min;
    command.rectColor.max = max;
    command.rectColor.z = z;
    command.rectColor.color = color;
    command.rectColor.transform = *transform;

    RenderQueuePush(queue, command);
}

void DrawQuad(RenderQueue* queue, v4 color, const m4x4* transform, v2 anchor) {
    RenderCommand command {};
    command.type = RenderCommandType::RectColor;
    command.rectColor.min = V2(-anchor.x, -anchor.y);
    command.rectColor.max = V2(1.0f - anchor.x, 1.0f - anchor.y);
    command.rectColor.z = 0.0f;
    command.rectColor.color = color;
    command.rectColor.transform = *transform;

    RenderQueuePush(queue, command);
}

void DrawLine(RenderQueue* queue, v3 begin, v3 end, v4 color) {
    RenderCommand command {};
    command.type = RenderCommandType::Line;
    command.line.begin = begin;
    command.line.end = end;
    command.line.color = color;

    RenderQueuePush(queue, command);
}
