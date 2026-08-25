#include <Windows.h>
#include <stdio.h>

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd)
{
    // OutputDebugStringA("Hello cpp");
    printf("Hello win32");
    fflush(stdout);

    return 0;
}
