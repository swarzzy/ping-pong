#include "SDLWin32Platform.h"

#include <SDL_opengl.h>

// TODO: Logger
void Logger(void* data, const char* fmt, va_list* args) {
    vprintf(fmt, *args);
}

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

LoggerFn* GlobalLogger = Logger;
void* GlobalLoggerData = nullptr;
AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

static Win32Context context;

void PollEvents(Win32Context* context) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: { context->running = false; } break;
        default: {} break;
        }
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
#if defined(ENABLE_CONSOLE)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    context.running = true;
    context.state.windowWidth = DefaultWindowWidth;
    context.state.windowHeight = DefaultWindowHeight;

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        panic("[SDL] Initialization failed: %s", SDL_GetError());
    }

#define sdl_gl_attrib_check(x)  if ((x) != 0) panic("[SDL] Failed to init OpnGL with attribute %s. %s", #x, SDL_GetError());

    // TODO(swarzzy): These attributes might be to strict for simple ping pong game
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1));

#undef sdl_gl_attrib_check

    auto windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    context.window = SDL_CreateWindow("Ping pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, context.state.windowWidth, context.state.windowHeight, windowFlags);
    if (!context.window) {
        panic("[SDL] Failed to create window: %s", SDL_GetError());
    }

    context.glContext = SDL_GL_CreateContext(context.window);
    if (!context.glContext) {
        panic("[SDL] Failed to initialize OpenGL context. %s", SDL_GetError());
    }

    if (SDL_GL_SetSwapInterval(1) != 0) {
        log_print("[SDL] Warning! V-sync is not supported\n");
    }

    while (context.running) {
        PollEvents(&context);

        glClearColor(1.0f, 0.4f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SDL_GL_SwapWindow(context.window);
    }

    // TODO(swarzzy): Is that necessary?
    SDL_Quit();
    return 0;
}
