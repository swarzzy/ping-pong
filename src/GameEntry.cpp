#include "Common.h"
#include "Platform.h"
#include "Math.h"
#include "platform/OpenGL.h"

// NOTE: Logger and assert handler implementation
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

// Setup logger and assert handler
LoggerFn* GlobalLogger = Logger;
void* GlobalLoggerData = nullptr;
AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

// Global variables for the game. They should be set every time after game code reloading
static PlatformState* _GlobalPlatformState;

// Shortcuts for OpenGL functions
#define gl_function(func) _GlobalPlatformState->gl->functions.fn. func

#define glClear gl_function(glClear)
#define glClearColor gl_function(glClearColor)
#define glViewport gl_function(glViewport)

// Shortcuts for platform functions
// For declarations see Platform.h
#define platform_call(func) _GlobalPlatformState->functions. func

#define PlatformDebugGetFileSize platform_call(DebugGetFileSize)
#define PlatformDebugReadFile platform_call(DebugReadFile)
#define PlatformDebugReadTextFile platform_call(DebugReadTextFile)
#define PlatformDebugWriteFile platform_call(DebugWriteFile)
#define PlatformDebugOpenFile platform_call(DebugOpenFile)
#define PlatformDebugCloseFile platform_call(DebugCloseFile)
#define PlatformDebugCopyFile platform_call(DebugCopyFile)
#define PlatformDebugWriteToOpenedFile platform_call(DebugWriteToOpenedFile)

// Allocator declaraions are in Common.h
#define PlatformAllocate platform_call(Allocate)
#define PlatformDeallocate platform_call(Deallocate)
#define PlatformReallocate platform_call(Reallocate)

#include "Game.h"

// Game context also should be set manually afted dll reloading
static GameContext* _GlobalGameContext;

const PlatformState* GetPlatform() { return _GlobalPlatformState; }
GameContext* GetContext() {return _GlobalGameContext; }
const InputState* GetInput() { return &_GlobalPlatformState->input; }


// NOTE: Game DLL entry point. Will be called by the platform layer.
extern "C" GAME_CODE_ENTRY void __cdecl GameUpdateAndRender(PlatformState* platform, GameInvoke invoke, void** data) {
    switch (invoke) {
    case GameInvoke::Init: {
        auto context = (GameContext*)platform->functions.Allocate(sizeof(GameContext), alignof(GameContext), nullptr);
        *data = context;
        _GlobalGameContext = context;
        _GlobalPlatformState = platform;
        GameInit();
    } break;
    case GameInvoke::Reload: {
        _GlobalGameContext = (GameContext*)*data;
        _GlobalPlatformState = platform;
        GameReload();
    } break;
    case GameInvoke::Update: {
        GameUpdate();
    } break;
    case GameInvoke::Render: {
        GameRender();
    } break;
    invalid_default();
    }
}

// NOTE(swarzzy): All game .cpp files should be included here
#include "Game.cpp"
