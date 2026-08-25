#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd)
{
    const char* greeting = "Hello world\n";
    OutputDebugString(greeting);

    char unsigned myname = 255;
    myname += 1;

    return 0;
}
