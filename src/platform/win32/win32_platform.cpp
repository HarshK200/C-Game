// pre complied header
#include "src/pch.h"

// utils
#include "src/utils/log.h"
#include "src/utils/constants.h"
#include "src/utils/arena_allocator.h"

#include "src/input/input.h"

// exposed struct definitions
#include "src/platform/win32/win32_platform.h"


// =================================================================================
//                              INTERNAL FUNCTIONS
// =================================================================================

void HandleKeyboardInput(InputManager* im, UINT message, WPARAM wparam)
{
    LOG_ASSERT(
        (message == WM_KEYDOWN || message == WM_KEYUP),
        "HandleKeyboardInput() called on invalid event. Make sure event == WM_KEYDOWN or WM_KEYUP");

    // NOTE(harsh): An action can only be transitioned to PRESSED if its in an IDLE state
    if (message == WM_KEYDOWN)
    {
        // LOG_INFO("WM_KEYDOWN");

        INPUT_ACTION action;
        bool known_action = true;
        switch (wparam)
        {
            case 'W':
                action = ACTION_MOVE_UP;
                break;
            case 'A':
                action = ACTION_MOVE_LEFT;
                break;
            case 'S':
                action = ACTION_MOVE_DOWN;
                break;
            case 'D':
                action = ACTION_MOVE_RIGHT;
                break;
            default:
                known_action = false;
                break;
        }
        if (known_action && im->ActionMap[action] == IDLE)
            im->ActionMap[action] = PRESSED;
    }

    if (message == WM_KEYUP)
    {
        // LOG_INFO("WM_KEYUP");
        switch (wparam)
        {
            case 'W':
                im->ActionMap[ACTION_MOVE_UP] = RELEASED;
                break;
            case 'A':
                im->ActionMap[ACTION_MOVE_LEFT] = RELEASED;
                break;
            case 'S':
                im->ActionMap[ACTION_MOVE_DOWN] = RELEASED;
                break;
            case 'D':
                im->ActionMap[ACTION_MOVE_RIGHT] = RELEASED;
                break;
            default:
                break;
        }
    }
}

void HandleMouseInput(InputManager* im, UINT message, WPARAM wparam)
{
    LOG_ASSERT(
        (message == WM_MOUSEMOVE ||
         message == WM_MOUSEWHEEL ||
         message == WM_MBUTTONDBLCLK ||
         message == WM_MBUTTONDOWN ||
         message == WM_MBUTTONUP),
        "HandleMouseInput() called on invalid event. Make sure event == WM_MOUSEMOVE or WM_MOUSEHWHEEL");


    // handle mouse wheel scroll
    if (message == WM_MOUSEWHEEL)
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        if (delta == 0)
            return;

        // scroll up
        if (delta > 0)
        {
            // LOG_INFO("SCROLLUP");
            im->ActionMap[ACTION_ZOOM_IN_DEBUG] = SINGLE_PRESSED;
        }

        // scroll down
        else if (delta < 0)
        {
            // LOG_INFO("SCROLL DOWN");
            im->ActionMap[ACTION_ZOOM_OUT_DEBUG] = SINGLE_PRESSED;
        }

        return;
    }

    // handle mouse button down
    if (message == WM_MBUTTONDOWN)
    {
        switch (wparam)
        {
            case MK_MBUTTON:
                im->ActionMap[ACTION_ZOOM_RESET_DEBUG] = PRESSED;
                break;
            default:
                break;
        }

        return;
    }

    // handle mouse button up
    if (message == WM_MBUTTONUP)
    {
        switch (wparam)
        {
            case MK_MBUTTON:
                im->ActionMap[ACTION_ZOOM_RESET_DEBUG] = RELEASED;
                break;
            default:
                break;
        }

        return;
    }

    if (message == WM_MOUSEMOVE)
    {

        return;
    }
}

/*
    Windows message callback. This function gets called everytime windows Dispatch's a message
    i.e. everytime DispatchMessage() is called.
*/
LRESULT CALLBACK WindowMessageCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    InputManager* input_manager = (InputManager*)GetWindowLongPtr(window, GWLP_USERDATA);

    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* create_info = (CREATESTRUCT*)lparam;
        input_manager = (InputManager*)create_info->lpCreateParams;
        SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)input_manager);
    }

    LRESULT result = 0;
    switch (message)
    {
        case WM_CREATE:
        {
            // LOG_INFO("WM_CREATE");
            break;
        }
        case WM_ACTIVATEAPP:
        {
            // LOG_INFO("WM_ACTIVATEAPP");
            break;
        }
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            HandleKeyboardInput(input_manager, message, wparam);
            break;
        }
        case WM_MOUSEWHEEL:
        case WM_MOUSEMOVE:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            HandleMouseInput(input_manager, message, wparam);
            break;
        }
        case WM_SIZE:
            // LOG_INFO("WM_SIZE");
            input_manager->WindowResized = true;
            break;
        case WM_CLOSE:
        {
            // LOG_INFO("WM_CLOSE");
            DestroyWindow(window);
            break;
        }
        case WM_DESTROY:
        {
            // LOG_INFO("WM_DESTROY");
            PostQuitMessage(0);
            break;
        }
        default:
        {
            result = DefWindowProc(window, message, wparam, lparam);
            break;
        }
    }

    return result;
}


// =================================================================================
//                      PLATFORM SERVICES FUNCTION DEFINITIONS
// =================================================================================

/*
    Creates a window using win32 api and returns the PlatformWindow* on success,
    otherwise returns nullptr on failure
*/
PlatformWindow* PlatformOpenWindow(AppMemory* memory, InputManager* input_manager)
{
    PlatformWindow* window = ArenaAlloc<PlatformWindow>(&memory->PermanentAllocator, sizeof(PlatformWindow));

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
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // use WS_POPUP instane of WS_OVERLAPPEDWINDOW for a boderless window
        CW_USEDEFAULT, CW_USEDEFAULT,
        outer_width, outer_height,
        NULL,
        NULL,
        instance,
        input_manager // This pointer i.e. in the lpParam field becomes available to WM_NCCREATE
    );
    if (!window->Handle)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
        return nullptr;
    }

    return window;
}

/*
    Proccess all the input events in the Window Message Queue
    If message is WM_QUIT returns 1, otherwise returns 0 on successful finish
*/
int PlatformProcessInput(InputManager* input_manager)
{
    for (int i = 0; i < INPUT_ACTION_COUNT; i++)
    {
        switch (input_manager->ActionMap[i])
        {
            case PRESSED:
                input_manager->ActionMap[i] = HELD;
                break;
            case HELD:
                break;
            case RELEASED:
                input_manager->ActionMap[i] = IDLE;
                break;
            case SINGLE_PRESSED:
                input_manager->ActionMap[i] = IDLE;
                break;
            default:
                break;
        }
    }

    MSG message;
    int result = 0;

    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
    {
        // TODO(harsh): maybe add error handling if translate and dispatch fails??
        TranslateMessage(&message);
        DispatchMessage(&message);

        if (message.message == WM_QUIT)
            result = 1;
    }

    return result;
}

void PlatformSleep(unsigned long ms)
{
    Sleep(ms);
}

void* PlatformLoadDynamicLibrary(const char* dll_path)
{
    HMODULE dll = LoadLibrary(dll_path);
    LOG_ASSERT(dll, "failed to load dll at path: %s", dll_path);

    return dll;
}

void* PlatformLoadDynamicFunction(void* dll, const char* func_name)
{
    FARPROC func_ptr = GetProcAddress((HMODULE)dll, func_name);
    LOG_ASSERT(dll, "failed to load function with name: %s", func_name);

    return (void*)func_ptr;
}

bool PlatformFreeDynamicLibrary(void* dll)
{
    bool free_result = FreeLibrary((HMODULE)dll);
    LOG_ASSERT(free_result, "Failed to free dynamic dll");

    return free_result;
}
