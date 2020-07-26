#include "Render.h"

const char* ColorRectShaderVertex = R"(
#version 330 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Color;

out vec4 VertexColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(Position.xyz, 1.0f);
    VertexColor = Color;
})";

const char* ColorRectShaderFragmentOpaque = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;

void main() {
    FragmentColor = vec4(VertexColor.xyz, 1.0f);
})";

const char* ColorRectShaderFragmentTransparent = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;

void main() {
    FragmentColor = VertexColor;
})";

const char* LineShaderVertex = R"(
#version 330 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Color;

out vec4 VertexColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(Position.xyz, 1.0f);
    VertexColor = Color;
})";

const char* LineShaderFragment = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;

void main() {
    FragmentColor = vec4(VertexColor.xyz, 1.0f);
})";

struct Vertex {
    v4 position;
    v4 color;
};

struct LineVertex {
    v4 position;
    v4 color;
};

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource);

void RendererReloadTargets(Renderer* renderer, uv2 newRes, u32 newSampleCount) {
    // TODO: There are could be a problems on some drivers
    // with changing framebuffer attachments so this code needs to be checked
    // on different GPUs and drivers
    if (newSampleCount <= renderer->maxSupportedSampleCount) {
        renderer->resolution = newRes;
        renderer->sampleCount = newSampleCount;
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->multisampledColorTarget);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, newSampleCount, GL_RGBA8, renderer->resolution.x, renderer->resolution.y, GL_FALSE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->multisampledDepthStencilTarget);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, newSampleCount, GL_DEPTH_STENCIL, renderer->resolution.x, renderer->resolution.y, GL_FALSE);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
}

void RendererInit(Renderer* renderer, uv2 resolution, u32 sampleCount) {
    GLuint globalVAO;
    glGenVertexArrays(1, &globalVAO);
    glBindVertexArray(globalVAO);

    GLint maxSamples = 1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    renderer->maxSupportedSampleCount = maxSamples;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    // TODO(swarzzy): Multisampling
    //glEnable(GL_MULTISAMPLE);


    glLineWidth(3.0f);

    glClearDepth(1.0f);

    glGenBuffers(1, &renderer->vertexBuffer);
    assert(renderer->vertexBuffer);

    glGenBuffers(1, &renderer->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u16) * Renderer::MaxBufferCapacity, nullptr, GL_STATIC_DRAW);
    u16* indexData = (u16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    static_assert((Renderer::MaxBufferCapacity % 6) == 0);

    usize k = 0;
    for (usize i = 0; i < Renderer::MaxBufferCapacity; i+= 6) {
        indexData[i + 0] = k;
        indexData[i + 1] = k + 1;
        indexData[i + 2] = k + 2;
        indexData[i + 3] = k + 2;
        indexData[i + 4] = k + 3;
        indexData[i + 5] = k;
        k += 4;
    }

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    renderer->rectColorOpaqueShader = CompileGLSL("RectColorOpaque", ColorRectShaderVertex, ColorRectShaderFragmentOpaque);
    assert(renderer->rectColorOpaqueShader);
    renderer->mvpLocation = glGetUniformLocation(renderer->rectColorOpaqueShader, "MVP");
    assert(renderer->mvpLocation != -1);

    renderer->rectColorTransparentShader = CompileGLSL("RectColorTransparent", ColorRectShaderVertex, ColorRectShaderFragmentTransparent);
    assert(renderer->rectColorTransparentShader);
    renderer->mvpLocationRectColorTransparent = glGetUniformLocation(renderer->rectColorTransparentShader, "MVP");
    assert(renderer->mvpLocationRectColorTransparent != -1);

    renderer->lineShader = CompileGLSL("LineShader", LineShaderVertex, LineShaderFragment);
    assert(renderer->lineShader);
    renderer->mvpLocationLine = glGetUniformLocation(renderer->lineShader, "MVP");
    assert(renderer->mvpLocationLine != -1);

    glGenFramebuffers(1, &renderer->multisampledBuffer);
    assert(renderer->multisampledBuffer);

    glGenTextures(1, &renderer->multisampledColorTarget);
    glGenTextures(1, &renderer->multisampledDepthStencilTarget);

    renderer->resolution = resolution;
    renderer->sampleCount = Min(sampleCount, renderer->maxSupportedSampleCount);
    RendererReloadTargets(renderer, resolution, renderer->sampleCount);

    glBindFramebuffer(GL_FRAMEBUFFER, renderer->multisampledBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderer->multisampledColorTarget, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, renderer->multisampledDepthStencilTarget, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, renderer->multisampledDepthStencilTarget, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RendererChangeResolution(Renderer* renderer, uv2 newRes, u32 newSampleCount) {
    // TODO(swarzzy): Check for max supported resolution
    renderer->resolution = newRes;
    renderer->sampleCount = Min(newSampleCount, renderer->maxSupportedSampleCount);
    RendererReloadTargets(renderer, renderer->resolution, renderer->sampleCount);
}

void RendererBeginFrame(Renderer* renderer) {
    glUseProgram(renderer->rectColorOpaqueShader);
    glUniformMatrix4fv(renderer->mvpLocation, 1, GL_FALSE, renderer->canvas.projection.data);

    glUseProgram(renderer->lineShader);
    glUniformMatrix4fv(renderer->mvpLocationLine, 1, GL_FALSE, renderer->canvas.projection.data);

    glUseProgram(renderer->rectColorTransparentShader);
    glUniformMatrix4fv(renderer->mvpLocationRectColorTransparent, 1, GL_FALSE, renderer->canvas.projection.data);

    const PlatformState* platform = GetPlatform();
    glViewport(0, 0, platform->windowWidth, platform->windowHeight);

    // Render to multisampled framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->multisampledBuffer);

    glClearColor(renderer->canvas.clearColor.r, renderer->canvas.clearColor.g, renderer->canvas.clearColor.b, renderer->canvas.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND); // nocheckin
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void RendererFlushRectQueue(Renderer* renderer, RenderQueue* queue) {
    auto opaqueBuffer = queue->buffers + RenderQueue::OpaqueQueue;
    auto transparentBuffer = queue->buffers + RenderQueue::TransparentQueue;
    if (opaqueBuffer->at || transparentBuffer->at) {
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);

        uptr vertexBufferSize = 4 * sizeof(Vertex) * (opaqueBuffer->at + transparentBuffer->at);
        // TODO(swarzzy): Is GL_STREAM_DRAW hint ok here
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_STREAM_DRAW);
        Vertex* vertexBuffer = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(vertexBuffer);

        usize bufferAt = 0;

        auto PushQuad = [&] (const RenderCommand* command) {
            // Premultiplied alpha
            v4 premulColor = V4(command->rectColor.color.rgb * command->rectColor.color.a, command->rectColor.color.a);

            vertexBuffer[bufferAt + 0].position = command->rectColor.transform * V4(command->rectColor.min, command->rectColor.z, 1.0f);
            vertexBuffer[bufferAt + 0].color = premulColor;

            vertexBuffer[bufferAt + 1].position = command->rectColor.transform * V4(command->rectColor.max.x, command->rectColor.min.y, command->rectColor.z, 1.0f);
            vertexBuffer[bufferAt + 1].color = premulColor;

            vertexBuffer[bufferAt + 2].position = command->rectColor.transform * V4(command->rectColor.max, command->rectColor.z, 1.0f);
            vertexBuffer[bufferAt + 2].color = premulColor;

            vertexBuffer[bufferAt + 3].position = command->rectColor.transform * V4(command->rectColor.min.x, command->rectColor.max.y, command->rectColor.z, 1.0f);
            vertexBuffer[bufferAt + 3].color = premulColor;

            bufferAt += 4;
        };

        for (usize i = 0; i < opaqueBuffer->at; i++) {
            auto command = opaqueBuffer->commands + i;

            // TODO(swarzzy): Only this command supported for now
            assert(command->type == RenderCommandType::RectColor);
            assert(command->transparent == 0);

            PushQuad(command);
        }

        for (usize i = 0; i < transparentBuffer->at; i++) {
            auto command = transparentBuffer->commands + i;

            // TODO(swarzzy): Only this command supported for now
            assert(command->type == RenderCommandType::RectColor);
            assert(command->transparent == 1);

            PushQuad(command);
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), (void*)sizeof(v4));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);

        glUseProgram(renderer->rectColorOpaqueShader);
        glDepthFunc(GL_LEQUAL);

        // Draw all opaque geometry
        glDrawElements(GL_TRIANGLES, opaqueBuffer->at * 6, GL_UNSIGNED_SHORT, 0);

        // Draw transparent geometry
        glUseProgram(renderer->rectColorTransparentShader);
        glDepthFunc(GL_ALWAYS);

        glDrawElements(GL_TRIANGLES, transparentBuffer->at * 6, GL_UNSIGNED_SHORT, (void*)(sizeof(u16) * opaqueBuffer->at * 6));
        //printf("%d\n", transparentBuffer->at);
    }
}

void RendererFlushLineQueue(Renderer* renderer, RenderQueue* queue) {
    auto buffer = queue->buffers + RenderQueue::LineQueue;
    if (buffer->at) {
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
        // TODO(swarzzy): Is GL_STREAM_DRAW hint ok here?
        glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * buffer->at * 2, nullptr, GL_STREAM_DRAW);
        Vertex* vertexBuffer = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(vertexBuffer);

        usize bufferAt = 0;

        for (usize i = 0; i < buffer->at; i++) {
            auto command = buffer->commands + i;

            // TODO(swarzzy): Only this command supported for now
            assert(command->type == RenderCommandType::Line);
            assert(command->transparent == false);

            vertexBuffer[bufferAt + 0].position = V4(command->line.begin, 1.0f);
            vertexBuffer[bufferAt + 0].color = command->line.color;

            vertexBuffer[bufferAt + 1].position = V4(command->line.end, 1.0f);
            vertexBuffer[bufferAt + 1].color = command->line.color;

            bufferAt += 2;
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), (void*)sizeof(v4));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glUseProgram(renderer->lineShader);

        glDrawArrays(GL_LINES, 0, buffer->at * 2);
    }
}

void RendererDraw(Renderer* renderer, RenderQueue* queue) {
    RendererFlushRectQueue(renderer, queue);
    RendererFlushLineQueue(renderer, queue);
}

void RendererEndFrame(Renderer* renderer) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->multisampledBuffer);

    // Resolve multisampled framebuffer and blit it to offsceen buffer
    glBlitFramebuffer(0, 0, renderer->resolution.x, renderer->resolution.y,
                      0, 0, renderer->resolution.x, renderer->resolution.y,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource) {
    GLuint resultHandle = 0;

    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    if (vertexHandle) {
        glShaderSource(vertexHandle, 1, &vertexSource, nullptr);
        glCompileShader(vertexHandle);

        GLint vertexResult = 0;
        glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &vertexResult);
        if (vertexResult) {
            GLuint fragmentHandle;
            fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);
            if (fragmentHandle) {
                glShaderSource(fragmentHandle, 1, &fragmentSource, nullptr);
                glCompileShader(fragmentHandle);

                GLint fragmentResult = 0;
                glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &fragmentResult);
                if (fragmentResult) {
                    GLint programHandle;
                    programHandle = glCreateProgram();
                    if (programHandle) {
                        glAttachShader(programHandle, vertexHandle);
                        glAttachShader(programHandle, fragmentHandle);
                        glLinkProgram(programHandle);

                        GLint linkResult = 0;
                        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkResult);
                        if (linkResult) {
                            glDeleteShader(vertexHandle);
                            glDeleteShader(fragmentHandle);
                            resultHandle = programHandle;
                        } else {
                            i32 logLength;
                            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
                            char* message = (char*)PlatformAllocate(logLength, 0, nullptr);
                            glGetProgramInfoLog(programHandle, logLength, 0, message);
                            log_print("[Error]: Failed to link shader program (%s) \n%s\n", name, message);
                            PlatformDeallocate(message, nullptr);
                        }
                    } else {
                        log_print("Failed to create shader program\n");
                    }
                } else {
                    GLint logLength;
                    glGetShaderiv(fragmentHandle, GL_INFO_LOG_LENGTH, &logLength);
                    char* message = (char*)PlatformAllocate(logLength, 0, nullptr);
                    glGetShaderInfoLog(fragmentHandle, logLength, nullptr, message);
                    log_print("[Error]: Failed to compile frag shader (%s)\n%s\n", name, message);
                    PlatformDeallocate(message, nullptr);
                }
            } else {
                log_print("Failed to create fragment shader\n");
            }
        } else {
            GLint logLength;
            glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength);
            char* message = (char*)PlatformAllocate(logLength, 0, nullptr);
            glGetShaderInfoLog(vertexHandle, logLength, nullptr, message);
            log_print("[Error]: Failed to compile vertex shader (%s)\n%s", name, message);
            PlatformDeallocate(message, nullptr);
        }
    } else {
        log_print("Falled to create vertex shader\n");
    }
    return resultHandle;
}
