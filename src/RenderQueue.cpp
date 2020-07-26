#include "RenderQueue.h"

void RenderQueueInit(RenderQueue* queue, u32 size) {
    *queue = {};

    for (auto& it : queue->buffers) {
        it.size = size;
        it.at = 0;
        it.commands = (RenderCommand*)PlatformAllocate(sizeof(RenderCommand) * size, 0, nullptr);
        assert(it.commands);
    }
}

bool RenderQueuePushInBuffer(RenderQueue* queue, const RenderCommand* command, u32 bufferIndex) {
    bool result = false;
    if (bufferIndex < RenderQueue::QueueCount) {
        auto buffer = queue->buffers + bufferIndex;
        if (buffer->at < buffer->size) {
            buffer->commands[buffer->at] = *command;
            buffer->at++;
            result = true;
        }
    }
    return result;
}

void RenderQueuePush(RenderQueue* queue, RenderCommand command) {
    switch (command.type) {
    case RenderCommandType::RectColor: {
        if (command.transparent) {
            RenderQueuePushInBuffer(queue, &command, RenderQueue::TransparentQueue);
        } else {
            RenderQueuePushInBuffer(queue, &command, RenderQueue::OpaqueQueue);
        }
    } break;
    case RenderCommandType::Line: {
        RenderQueuePushInBuffer(queue, &command, RenderQueue::LineQueue);
    } break;
    invalid_default();
    }
}

void RenderQueuePrepare(RenderQueue* queue) {
    if (queue->buffers[RenderQueue::TransparentQueue].at) {
        auto buffer = queue->buffers + RenderQueue::TransparentQueue;
        auto pred = [](const void* _a, const void* _b) {
            auto a = (RenderCommand*)_a;
            auto b = (RenderCommand*)_b;
            // nocheckin
            // Commands other than RectColor
            if (a->rectColor.z < b->rectColor.z) return -1;
            if (a->rectColor.z == b->rectColor.z) return 0;
            if (a->rectColor.z > b->rectColor.z) return 1;
            return 0;
        };

        // TODO(swarzzy): This is super inefficient for now. In the future we need
        // to store commands and their data separately and sort only commands
        qsort(buffer->commands, buffer->at, sizeof(RenderCommand), pred);
    }
}

void RenderQueueReset(RenderQueue* queue) {
    for (auto& it : queue->buffers) {
        it.at = 0;
    }
}

void DrawQuad(RenderQueue* queue, v2 min, v2 max, f32 z, v4 color) {
    RenderCommand command {};
    command.type = RenderCommandType::RectColor;
    command.transparent = color.a == 1.0f ? false : true;
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
    command.transparent = color.a == 1.0f ? false : true;
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
    command.transparent = color.a == 1.0f ? false : true;
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
    //command.transparent = color.a == 1.0f ? false : true;
    command.line.begin = begin;
    command.line.end = end;
    command.line.color = color;

    RenderQueuePush(queue, command);
}
