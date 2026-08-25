#include <Windows.h>

void foo()
{
    const char *greeting = "Hello world\n";
    OutputDebugString(greeting);
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd)
{
    foo();

    return 0;
}
