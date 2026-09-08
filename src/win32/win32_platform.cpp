#include "src/utils/arena_allocator.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


#include "src/utils/constants.h"
#include "src/utils/log.h"
#include "src/win32/win32_platform.h"
#include "src/win32/win32_input.h"


// ================== Platform Layer Services Definitions ==================

/*
    Creates a window using win32 api and returns the PlatformWindow* on success,
    otherwise returns nullptr on failure
*/
PlatformWindow* PlatformOpenWindow(AppMemory* memory)
{
    PlatformWindow* window = (PlatformWindow*)ArenaAlloc(&memory->PermanentAllocator, sizeof(PlatformWindow));

    HINSTANCE instance = GetModuleHandle(0);
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = InputWindowCallback;
    window_class.hInstance = instance;
    // TODO(harsh): add custom icon and cursor for the game
    window_class.hIcon = LoadIcon(instance, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = "isekaied_class";
    if (RegisterClass(&window_class) == 0)
    {
        LOG_ERROR("Registering window failed.");
        return nullptr;
    }
    window->Handle = CreateWindowEx(
        NULL,
        window_class.lpszClassName,
        "isekaied",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        DEFAULT_WINDOW_RESOLUTION.x,
        DEFAULT_WINDOW_RESOLUTION.y,
        NULL,
        NULL,
        instance,
        NULL);
    if (!window->Handle)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
        return nullptr;
    }

    return window;
}
