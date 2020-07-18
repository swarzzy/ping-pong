#include "SDLWin32Platform.h"

#include <SDL_opengl.h>
#include <SDL_keycode.h>

// Forcing the app to use discrete graphics adpter by default
#if defined (DISCRETE_GRAPHICS_DEFAULT)
extern "C" { __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01; }
#endif

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

static Win32Context GlobalContext;
static void* GlobalGameData;

Key SDLKeycodeConvert(i32 sdlKeycode);
MouseButton SDLMouseButtonConvert(u8 button);

void GatherMouseMovement(Win32Context* context) {
    if (context->state.input.activeApp) {
        int mousePositionX;
        int mousePositionY;

        // TODO(swarzzy): This function in not what I actually need. It gets mouse
        // coordinates from event messgaes, but I want to use immediate OS getter
        // (i.e. GetCursorPos (SDL version: SDL_GetGlobalMouseState)). But GetCursorPos
        // returns screen-relative coordinates and i need window-relative.
        // So for now just go with SDL_GetMouseState.
        SDL_GetMouseState(&mousePositionX, &mousePositionY);

        if ((mousePositionX != context->mousePosX) || (mousePositionY != context->mousePosY)) {
            i32 offsetX = mousePositionX - context->mousePosX;
            i32 offsetY = -(mousePositionY - context->mousePosY);

            context->state.input.mouseFrameOffsetX = offsetX / (f32)context->state.windowWidth;
            context->state.input.mouseFrameOffsetY = offsetY / (f32)context->state.windowHeight;

            context->mousePosX = mousePositionX;
            context->mousePosY = mousePositionY;
            context->state.input.mouseX = (f32)mousePositionX;
            context->state.input.mouseY = (f32)mousePositionY;
            context->state.input.mouseX = mousePositionX / (f32)context->state.windowWidth;
            context->state.input.mouseY = (context->state.windowHeight - mousePositionY) / (f32)context->state.windowHeight;
        }
    }
}

void PollEvents(Win32Context* context) {

    GatherMouseMovement(context);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_WINDOWEVENT: {
            switch (event.window.event) {

            case SDL_WINDOWEVENT_SIZE_CHANGED: {
                context->state.windowWidth = event.window.data1;
                context->state.windowHeight = event.window.data2;
            } break;

            case SDL_WINDOWEVENT_ENTER: {
                context->state.input.mouseInWindow = true;
            } break;

            case SDL_WINDOWEVENT_LEAVE: {
                context->state.input.mouseInWindow = false;
            } break;

            case SDL_WINDOWEVENT_FOCUS_GAINED: {
                // TODO(swarzzy): This probably does not correspond to win32 api
                // implementation of platform layer. In win32 this flag is toggled
                // on WM_ACTIVEAPP event, but it seems like SDL throws this event
                // on WM_ACTIVE. SDL doc says that this event is for keyboard focus
                // [https://wiki.libsdl.org/SDL_WindowEventID]
                context->state.input.activeApp = true;
            } break;

            case SDL_WINDOWEVENT_FOCUS_LOST: {
                context->state.input.activeApp = false;
            } break;

            default: {} break;
            }
        } break;

        case SDL_MOUSEBUTTONDOWN: {
            MouseButton button = SDLMouseButtonConvert(event.button.button);
            context->state.input.mouseButtons[(u32)button].wasPressed =  context->state.input.mouseButtons[(u32)button].pressedNow;
            context->state.input.mouseButtons[(u32)button].pressedNow = true;
            //printf("Mouse button pressed: %s\n", ToString(button));
        } break;

        case SDL_MOUSEBUTTONUP: {
            MouseButton button = SDLMouseButtonConvert(event.button.button);
            context->state.input.mouseButtons[(u32)button].wasPressed =  context->state.input.mouseButtons[(u32)button].pressedNow;
            context->state.input.mouseButtons[(u32)button].pressedNow = false;
            //printf("Mouse button released: %s\n", ToString(button));
        } break;

        case SDL_MOUSEWHEEL: {
            // TODO(swarzzy): Check for SDL_MOUSEWHEEL_NORMAL and SDL_MOUSEWHEEL_FLIPPED
            // TODO(swarzzy): x!
            i32 steps = event.wheel.y;
            context->state.input.scrollOffset = steps;
            context->state.input.scrollFrameOffset = steps;
        } break;

        case SDL_KEYDOWN: {
            // TODO(swarzzy): Is SDL can throw KEYDOWN event which has state SDL_RELEASED???
            // We need test this
            assert(event.key.state == SDL_PRESSED);

            // TODO(swarzzy): What it the real difference between scancodes and keycodes
            // and which of them we sould use

            // TODO(swarzzy): Repeat count

            u32 keyCode = (u32)SDLKeycodeConvert(event.key.keysym.sym);
            u16 repeatCount =  event.key.repeat;
            context->state.input.keys[keyCode].wasPressed = context->state.input.keys[keyCode].pressedNow;
            context->state.input.keys[keyCode].pressedNow = true;
        } break;

        case SDL_KEYUP: {
            assert(event.key.state == SDL_RELEASED);

            u32 keyCode = (u32)SDLKeycodeConvert(event.key.keysym.sym);
            u16 repeatCount =  event.key.repeat;
            context->state.input.keys[keyCode].wasPressed = context->state.input.keys[keyCode].pressedNow;
            context->state.input.keys[keyCode].pressedNow = false;
        } break;

        case SDL_QUIT: { context->running = false; } break;
        default: {} break;
        }
    }
}

f64 GetTimeStamp() {
    f64 time = 0.0;
    LARGE_INTEGER currentTime = {};
    if (QueryPerformanceCounter(&currentTime)) {
        time = (f64)(currentTime.QuadPart) / (f64)GlobalContext.performanceFrequency.QuadPart;
    }
    return time;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
#if defined(ENABLE_CONSOLE)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    auto context = &GlobalContext;

    // NOTE(swarzzy): Setting granularity of windows scheduler
    // so Sleep will work with more granular value
    UINT sleepGranularityMs = 1;
    auto granularityWasSet = (timeBeginPeriod(sleepGranularityMs) == TIMERR_NOERROR);

    QueryPerformanceFrequency(&context->performanceFrequency);

    context->running = true;
    context->state.windowWidth = DefaultWindowWidth;
    context->state.windowHeight = DefaultWindowHeight;

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        panic("[SDL] Initialization failed: %s", SDL_GetError());
    }

#define sdl_gl_attrib_check(x)  if ((x) != 0) panic("[SDL] Failed to init OpneGL with attribute %s. %s", #x, SDL_GetError());

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
    context->window = SDL_CreateWindow("Ping pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, context->state.windowWidth, context->state.windowHeight, windowFlags);
    if (!context->window) {
        panic("[SDL] Failed to create window: %s", SDL_GetError());
    }

    context->glContext = SDL_GL_CreateContext(context->window);
    if (!context->glContext) {
        panic("[SDL] Failed to initialize OpenGL context-> %s", SDL_GetError());
    }

    bool vSyncSupported = true;
    if (SDL_GL_SetSwapInterval(1) != 0) {
        log_print("[SDL] Warning! V-sync is not supported\n");
        vSyncSupported = false;
    }

    if (!UpdateGameCode(&context->gameLib)) {
        panic("[Platform] Failed to load game library");
    }

    // Init the game
    context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Init, &GlobalGameData);

    while (context->running) {
        auto frameStartTime = GetTimeStamp();
        context->state.tickCount++;

        // Reload game lib if it was updated
        bool codeReloaded = UpdateGameCode(&context->gameLib);
        if (codeReloaded) {
            context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Reload, &GlobalGameData);
        }

        // TODO(swarzzy): For now we do ONE update per frame with variable delta time.
        // This is not a good solution. We probably need to do updates with fixed timestep
        // with higher frequency (for example 120Hz)

        PollEvents(context);

        context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Update, &GlobalGameData);

        for (u32 keyIndex = 0; keyIndex < InputState::KeyCount; keyIndex ++) {
            context->state.input.keys[keyIndex].wasPressed = context->state.input.keys[keyIndex].pressedNow;
        }

        for (u32 mbIndex = 0; mbIndex < InputState::MouseButtonCount; mbIndex++) {
            context->state.input.mouseButtons[mbIndex].wasPressed = context->state.input.mouseButtons[mbIndex].pressedNow;
        }

        context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Render, &GlobalGameData);

        SDL_GL_SwapWindow(context->window);

        auto frameEndTime = GetTimeStamp();
        auto frameTime = frameEndTime - frameStartTime;

        // If framerate lower than 15 fps just clamping delta time
        context->state.deltaTime = (f32)Clamp(1.0 / frameTime, 0.0, 0.066);
        context->state.fps = (i32)(1.0f / context->state.deltaTime);
        context->state.ups = context->state.fps;
    }

    // TODO(swarzzy): Is that necessary?
    SDL_Quit();
    return 0;
}

Key SDLKeycodeConvert(i32 sdlKeycode) {
    // TODO(swarzzy): Test this
    switch (sdlKeycode) {
    case SDLK_BACKSPACE:    return Key::Backspace;
    case SDLK_TAB:          return Key::Tab;
    case SDLK_CLEAR:        return Key::Clear;
    case SDLK_RETURN:       return Key::Enter;
    case SDLK_LSHIFT:       return Key::LeftShift;
    case SDLK_LCTRL:        return Key::LeftCtrl;
    case SDLK_LALT:         return Key::LeftAlt;
    case SDLK_RSHIFT:       return Key::RightShift;
    case SDLK_RCTRL:        return Key::RightCtrl;
    case SDLK_RALT:         return Key::RightAlt;
    case SDLK_PAUSE:        return Key::Pause;
    case SDLK_CAPSLOCK:     return Key::CapsLock;
    case SDLK_ESCAPE:       return Key::Escape;
    case SDLK_SPACE:        return Key::Space;
    case SDLK_PAGEUP:       return Key::PageUp;
    case SDLK_PAGEDOWN:     return Key::PageDown;
    case SDLK_END:          return Key::End;
    case SDLK_HOME:         return Key::Home;
    case SDLK_LEFT:         return Key::Left;
    case SDLK_UP:           return Key::Up;
    case SDLK_RIGHT:        return Key::Right;
    case SDLK_DOWN:         return Key::Down;
    case SDLK_PRINTSCREEN:  return Key::PrintScreen;
    case SDLK_INSERT:       return Key::Insert;
    case SDLK_DELETE:       return Key::Delete;
    case SDLK_0:            return Key::_0;
    case SDLK_1:            return Key::_1;
    case SDLK_2:            return Key::_2;
    case SDLK_3:            return Key::_3;
    case SDLK_4:            return Key::_4;
    case SDLK_5:            return Key::_5;
    case SDLK_6:            return Key::_6;
    case SDLK_7:            return Key::_7;
    case SDLK_8:            return Key::_8;
    case SDLK_9:            return Key::_9;
    case SDLK_a:            return Key::A;
    case SDLK_b:            return Key::B;
    case SDLK_c:            return Key::C;
    case SDLK_d:            return Key::D;
    case SDLK_e:            return Key::E;
    case SDLK_f:            return Key::F;
    case SDLK_g:            return Key::G;
    case SDLK_h:            return Key::H;
    case SDLK_i:            return Key::I;
    case SDLK_j:            return Key::J;
    case SDLK_k:            return Key::K;
    case SDLK_l:            return Key::L;
    case SDLK_m:            return Key::M;
    case SDLK_n:            return Key::N;
    case SDLK_o:            return Key::O;
    case SDLK_p:            return Key::P;
    case SDLK_q:            return Key::Q;
    case SDLK_r:            return Key::R;
    case SDLK_s:            return Key::S;
    case SDLK_t:            return Key::T;
    case SDLK_u:            return Key::U;
    case SDLK_v:            return Key::V;
    case SDLK_w:            return Key::W;
    case SDLK_x:            return Key::X;
    case SDLK_y:            return Key::Y;
    case SDLK_z:            return Key::Z;
    case SDLK_LGUI:         return Key::LeftSuper;
    case SDLK_RGUI:         return Key::RightSuper;
    case SDLK_KP_0:         return Key::Num0;
    case SDLK_KP_1:         return Key::Num1;
    case SDLK_KP_2:         return Key::Num2;
    case SDLK_KP_3:         return Key::Num3;
    case SDLK_KP_4:         return Key::Num4;
    case SDLK_KP_5:         return Key::Num5;
    case SDLK_KP_6:         return Key::Num6;
    case SDLK_KP_7:         return Key::Num7;
    case SDLK_KP_8:         return Key::Num8;
    case SDLK_KP_9:         return Key::Num9;
    case SDLK_KP_MULTIPLY:  return Key::NumMultiply;
    case SDLK_KP_PLUS:      return Key::NumAdd;
    case SDLK_KP_MINUS:     return Key::NumSubtract;
    case SDLK_KP_DIVIDE:    return Key::NumDivide;
    case SDLK_KP_ENTER:     return Key::NumEnter;
    // They are the same
    case SDLK_KP_DECIMAL:   return Key::NumDecimal;
    case SDLK_KP_PERIOD:    return Key::NumDecimal;

    case SDLK_F1:           return Key::F1;
    case SDLK_F2:           return Key::F2;
    case SDLK_F3:           return Key::F3;
    case SDLK_F4:           return Key::F4;
    case SDLK_F5:           return Key::F5;
    case SDLK_F6:           return Key::F6;
    case SDLK_F7:           return Key::F7;
    case SDLK_F8:           return Key::F8;
    case SDLK_F9:           return Key::F9;
    case SDLK_F10:          return Key::F10;
    case SDLK_F11:          return Key::F11;
    case SDLK_F12:          return Key::F12;
    case SDLK_F13:          return Key::F13;
    case SDLK_F14:          return Key::F14;
    case SDLK_F15:          return Key::F15;
    case SDLK_F16:          return Key::F16;
    case SDLK_F17:          return Key::F17;
    case SDLK_F18:          return Key::F18;
    case SDLK_F19:          return Key::F19;
    case SDLK_F20:          return Key::F20;
    case SDLK_F21:          return Key::F21;
    case SDLK_F22:          return Key::F22;
    case SDLK_F23:          return Key::F23;
    case SDLK_F24:          return Key::F24;
    case SDLK_NUMLOCKCLEAR: return Key::NumLock;
    case SDLK_SCROLLLOCK:   return Key::ScrollLock;
    case SDLK_APPLICATION:  return Key::Menu;
    case SDLK_SEMICOLON:    return Key::Semicolon;
    case SDLK_EQUALS:       return Key::Equal;
    case SDLK_COMMA:        return Key::Comma;
    case SDLK_MINUS:        return Key::Minus;
    case SDLK_PERIOD:       return Key::Period;
    case SDLK_SLASH:        return Key::Slash;
    case SDLK_BACKQUOTE:    return Key::Tilde;
    case SDLK_LEFTBRACKET:  return Key::LeftBracket;
    case SDLK_BACKSLASH:    return Key::BackSlash;
    case SDLK_RIGHTBRACKET: return Key::RightBracket;
    case SDLK_QUOTE:        return Key::Apostrophe;
    default:                return Key::Invalid;
    }
}

const char* ToString(Key keycode) {
    switch (keycode) {
    case Key::Backspace: { return "Backspace";  }
    case Key::Tab: { return "Tab";  }
    case Key::Clear: { return "Clear";  }
    case Key::Enter: { return "Enter";  }
    case Key::LeftShift: { return "LeftShift";  }
    case Key::LeftCtrl: { return "LeftCtrl";  }
    case Key::LeftAlt: { return "LeftAlt";  }
    case Key::RightShift: { return "RightShift";  }
    case Key::RightCtrl: { return "RightCtrl";  }
    case Key::RightAlt: { return "RightAlt";  }
    case Key::Pause: { return "Pause";  }
    case Key::CapsLock: { return "CapsLock";  }
    case Key::Escape: { return "Escape";  }
    case Key::Space: { return "Space";  }
    case Key::PageUp: { return "PageUp";  }
    case Key::PageDown: { return "PageDown";  }
    case Key::End: { return "End";  }
    case Key::Home: { return "Home";  }
    case Key::Left: { return "Left";  }
    case Key::Up: { return "Up";  }
    case Key::Right: { return "Right";  }
    case Key::Down: { return "Down";  }
    case Key::PrintScreen: { return "PrintScreen";  }
    case Key::Insert: { return "Insert";  }
    case Key::Delete: { return "Delete";  }
    case Key::_0: { return "0";  }
    case Key::_1: { return "1";  }
    case Key::_2: { return "2";  }
    case Key::_3: { return "3";  }
    case Key::_4: { return "4";  }
    case Key::_5: { return "5";  }
    case Key::_6: { return "6";  }
    case Key::_7: { return "7";  }
    case Key::_8: { return "8";  }
    case Key::_9: { return "9";  }
    case Key::A: { return "A";  }
    case Key::B: { return "B";  }
    case Key::C: { return "C";  }
    case Key::D: { return "D";  }
    case Key::E: { return "E";  }
    case Key::F: { return "F";  }
    case Key::G: { return "G";  }
    case Key::H: { return "H";  }
    case Key::I: { return "I";  }
    case Key::J: { return "J";  }
    case Key::K: { return "K";  }
    case Key::L: { return "L";  }
    case Key::M: { return "M";  }
    case Key::N: { return "N";  }
    case Key::O: { return "O";  }
    case Key::P: { return "P";  }
    case Key::Q: { return "Q";  }
    case Key::R: { return "R";  }
    case Key::S: { return "S";  }
    case Key::T: { return "T";  }
    case Key::U: { return "U";  }
    case Key::V: { return "V";  }
    case Key::W: { return "W";  }
    case Key::X: { return "X";  }
    case Key::Y: { return "Y";  }
    case Key::Z: { return "Z";  }
    case Key::LeftSuper: { return "LeftSuper";  }
    case Key::RightSuper: { return "RightSuper";  }
    case Key::Num0: { return "Num0";  }
    case Key::Num1: { return "Num1";  }
    case Key::Num2: { return "Num2";  }
    case Key::Num3: { return "Num3";  }
    case Key::Num4: { return "Num4";  }
    case Key::Num5: { return "Num5";  }
    case Key::Num6: { return "Num6";  }
    case Key::Num7: { return "Num7";  }
    case Key::Num8: { return "Num8";  }
    case Key::Num9: { return "Num9";  }
    case Key::NumMultiply: { return "NumMultiply";  }
    case Key::NumAdd: { return "NumAdd";  }
    case Key::NumSubtract: { return "NumSubtract";  }
    case Key::NumDecimal: { return "NumDecimal";  }
    case Key::NumDivide: { return "NumDivide";  }
    case Key::F1: { return "F1";  }
    case Key::F2: { return "F2";  }
    case Key::F3: { return "F3";  }
    case Key::F4: { return "F4";  }
    case Key::F5: { return "F5";  }
    case Key::F6: { return "F6";  }
    case Key::F7: { return "F7";  }
    case Key::F8: { return "F8";  }
    case Key::F9: { return "F9";  }
    case Key::F10: { return "F10";  }
    case Key::F11: { return "F11";  }
    case Key::F12: { return "F12";  }
    case Key::F13: { return "F13";  }
    case Key::F14: { return "F14";  }
    case Key::F15: { return "F15";  }
    case Key::F16: { return "F16";  }
    case Key::F17: { return "F17";  }
    case Key::F18: { return "F18";  }
    case Key::F19: { return "F19";  }
    case Key::F20: { return "F20";  }
    case Key::F21: { return "F21";  }
    case Key::F22: { return "F22";  }
    case Key::F23: { return "F23";  }
    case Key::F24: { return "F24";  }
    case Key::NumLock: { return "NumLock";  }
    case Key::ScrollLock: { return "ScrollLock";  }
    case Key::Menu: { return "Menu";  }
    case Key::Semicolon: { return "Semicolon";  }
    case Key::Equal: { return "Equal";  }
    case Key::Comma: { return "Comma";  }
    case Key::Minus: { return "Minus";  }
    case Key::Period: { return "Period";  }
    case Key::Slash: { return "Slash";  }
    case Key::Tilde: { return "Tilde";  }
    case Key::LeftBracket: { return "LeftBracket";  }
    case Key::BackSlash: { return "BackSlash";  }
    case Key::RightBracket: { return "RightBracket";  }
    case Key::Apostrophe: { return "Apostrophe";  }
    case Key::NumEnter: { return "NumEnter";  }
    default: { return "Invalid"; }
    }
}

MouseButton SDLMouseButtonConvert(u8 button) {
    switch (button) {
    case SDL_BUTTON_LEFT:   return MouseButton::Left;
    case SDL_BUTTON_RIGHT:  return MouseButton::Right;
    case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
    case SDL_BUTTON_X1:     return MouseButton::XButton1;
    case SDL_BUTTON_X2:     return MouseButton::XButton2;
    invalid_default();
    }
    // Dummy return for the compiler
    return MouseButton::Left;
}

const char* ToString(MouseButton button) {
    switch (button) {
    case MouseButton::Left:     return "Left";
    case MouseButton::Right:    return "Right";
    case MouseButton::Middle:   return "Middle";
    case MouseButton::XButton1: return "XButton1";
    case MouseButton::XButton2: return "XButton2";
        invalid_default();
    }
    // Dummy return for the compiler
    return "Unknown";
}

#include "Win32CodeLoader.cpp"
