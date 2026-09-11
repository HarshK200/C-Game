#include "src/utils/log.h"
#include "src/utils/globals.h"
#include "src/win32/win32_input.h"
#include "src/win32/win32_platform.h"
#include "src/utils/arena_allocator.h"


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
    window_class.lpfnWndProc = WindowMessageCallback;
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

    // NOTE(harsh): VERY IMPORTANT!! Calculate the outer window size based on the
    // window style we are using i.e. this will account for the drawable window rect +
    // style like border, menu, etc...
    DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT window_rect = {
        0,
        0,
        (LONG)DEFAULT_WINDOW_RESOLUTION.x,
        (LONG)DEFAULT_WINDOW_RESOLUTION.y,
    };
    AdjustWindowRect(&window_rect, window_style, FALSE);
    int outer_width = window_rect.right - window_rect.left;
    int outer_height = window_rect.bottom - window_rect.top;

    window->Handle = CreateWindowEx(
        NULL,
        window_class.lpszClassName,
        "isekaied",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        outer_width, outer_height,
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
