#include "RenderQueue.h"

void RenderQueueInit(RenderQueue* queue, u32 size) {
    queue->size = size;
    queue->at = 0;
    queue->buffer = (RenderCommand*)PlatformAllocate(sizeof(RenderCommand), 0, nullptr);
    assert(queue->buffer);
}

void RenderQueuePush(RenderQueue* queue, RenderCommand command) {
    if (queue->at < queue->size) {
        queue->buffer[queue->at] = command;
        queue->at++;
    }
}

void RenderQueueReset(RenderQueue* queue) {
    queue->at = 0;
}
