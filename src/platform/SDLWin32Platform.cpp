#include "SDLWin32Platform.h"

#include <SDL_opengl.h>
#include <SDL_keycode.h>

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

Key SDLKeyCodeConvert(i32 sdlKeycode) {
    // TODO(swarzzy): Test this
    switch (sdlKeycode) {
    case SDLK_BACKSPACE: { return Key::Backspace;  }
    case SDLK_TAB: { return Key::Tab;  }
    case SDLK_CLEAR: { return Key::Clear;  }
    case SDLK_RETURN: { return Key::Enter;  }
    case SDLK_LSHIFT: { return Key::LeftShift;  }
    case SDLK_LCTRL: { return Key::LeftCtrl;  }
    case SDLK_LALT: { return Key::LeftAlt;  }
    case SDLK_RSHIFT: { return Key::RightShift;  }
    case SDLK_RCTRL: { return Key::RightCtrl;  }
    case SDLK_RALT: { return Key::RightAlt;  }
    case SDLK_PAUSE: { return Key::Pause;  }
    case SDLK_CAPSLOCK: { return Key::CapsLock;  }
    case SDLK_ESCAPE: { return Key::Escape;  }
    case SDLK_SPACE: { return Key::Space;  }
    case SDLK_PAGEUP: { return Key::PageUp;  }
    case SDLK_PAGEDOWN: { return Key::PageDown;  }
    case SDLK_END: { return Key::End;  }
    case SDLK_HOME: { return Key::Home;  }
    case SDLK_LEFT: { return Key::Left;  }
    case SDLK_UP: { return Key::Up;  }
    case SDLK_RIGHT: { return Key::Right;  }
    case SDLK_DOWN: { return Key::Down;  }
    case SDLK_PRINTSCREEN: { return Key::PrintScreen;  }
    case SDLK_INSERT: { return Key::Insert;  }
    case SDLK_DELETE: { return Key::Delete;  }
    case SDLK_0: { return Key::_0;  }
    case SDLK_1: { return Key::_1;  }
    case SDLK_2: { return Key::_2;  }
    case SDLK_3: { return Key::_3;  }
    case SDLK_4: { return Key::_4;  }
    case SDLK_5: { return Key::_5;  }
    case SDLK_6: { return Key::_6;  }
    case SDLK_7: { return Key::_7;  }
    case SDLK_8: { return Key::_8;  }
    case SDLK_9: { return Key::_9;  }
    case SDLK_a: { return Key::A;  }
    case SDLK_b: { return Key::B;  }
    case SDLK_c: { return Key::C;  }
    case SDLK_d: { return Key::D;  }
    case SDLK_e: { return Key::E;  }
    case SDLK_f: { return Key::F;  }
    case SDLK_g: { return Key::G;  }
    case SDLK_h: { return Key::H;  }
    case SDLK_i: { return Key::I;  }
    case SDLK_j: { return Key::J;  }
    case SDLK_k: { return Key::K;  }
    case SDLK_l: { return Key::L;  }
    case SDLK_m: { return Key::M;  }
    case SDLK_n: { return Key::N;  }
    case SDLK_o: { return Key::O;  }
    case SDLK_p: { return Key::P;  }
    case SDLK_q: { return Key::Q;  }
    case SDLK_r: { return Key::R;  }
    case SDLK_s: { return Key::S;  }
    case SDLK_t: { return Key::T;  }
    case SDLK_u: { return Key::U;  }
    case SDLK_v: { return Key::V;  }
    case SDLK_w: { return Key::W;  }
    case SDLK_x: { return Key::X;  }
    case SDLK_y: { return Key::Y;  }
    case SDLK_z: { return Key::Z;  }
    case SDLK_LGUI: { return Key::LeftSuper;  }
    case SDLK_RGUI: { return Key::RightSuper;  }
    case SDLK_KP_0: { return Key::Num0;  }
    case SDLK_KP_1: { return Key::Num1;  }
    case SDLK_KP_2: { return Key::Num2;  }
    case SDLK_KP_3: { return Key::Num3;  }
    case SDLK_KP_4: { return Key::Num4;  }
    case SDLK_KP_5: { return Key::Num5;  }
    case SDLK_KP_6: { return Key::Num6;  }
    case SDLK_KP_7: { return Key::Num7;  }
    case SDLK_KP_8: { return Key::Num8;  }
    case SDLK_KP_9: { return Key::Num9;  }
    case SDLK_KP_MULTIPLY: { return Key::NumMultiply;  }
    case SDLK_KP_PLUS: { return Key::NumAdd;  }
    case SDLK_KP_MINUS: { return Key::NumSubtract;  }
    case SDLK_KP_DECIMAL: { return Key::NumDecimal;  }
    case SDLK_KP_DIVIDE: { return Key::NumDivide;  }
    case SDLK_F1: { return Key::F1;  }
    case SDLK_F2: { return Key::F2;  }
    case SDLK_F3: { return Key::F3;  }
    case SDLK_F4: { return Key::F4;  }
    case SDLK_F5: { return Key::F5;  }
    case SDLK_F6: { return Key::F6;  }
    case SDLK_F7: { return Key::F7;  }
    case SDLK_F8: { return Key::F8;  }
    case SDLK_F9: { return Key::F9;  }
    case SDLK_F10: { return Key::F10;  }
    case SDLK_F11: { return Key::F11;  }
    case SDLK_F12: { return Key::F12;  }
    case SDLK_F13: { return Key::F13;  }
    case SDLK_F14: { return Key::F14;  }
    case SDLK_F15: { return Key::F15;  }
    case SDLK_F16: { return Key::F16;  }
    case SDLK_F17: { return Key::F17;  }
    case SDLK_F18: { return Key::F18;  }
    case SDLK_F19: { return Key::F19;  }
    case SDLK_F20: { return Key::F20;  }
    case SDLK_F21: { return Key::F21;  }
    case SDLK_F22: { return Key::F22;  }
    case SDLK_F23: { return Key::F23;  }
    case SDLK_F24: { return Key::F24;  }
    case SDLK_NUMLOCKCLEAR: { return Key::NumLock;  }
    case SDLK_SCROLLLOCK: { return Key::ScrollLock;  }
    case SDLK_APPLICATION: { return Key::Menu;  }
    case SDLK_SEMICOLON: { return Key::Semicolon;  }
    case SDLK_EQUALS: { return Key::Equal;  }
    case SDLK_COMMA: { return Key::Comma;  }
    case SDLK_MINUS: { return Key::Minus;  }
    case SDLK_PERIOD: { return Key::Period;  }
    case SDLK_SLASH: { return Key::Slash;  }
    case SDLK_BACKQUOTE: { return Key::Tilde;  }
    case SDLK_LEFTBRACKET: { return Key::LeftBracket;  }
    case SDLK_BACKSLASH: { return Key::BackSlash;  }
    case SDLK_RIGHTBRACKET: { return Key::RightBracket;  }
    case SDLK_QUOTE: { return Key::Apostrophe;  }
    default: { return Key::Invalid; }
    }
}
