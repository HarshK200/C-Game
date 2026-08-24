#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nShowCmd)
{
    MessageBox(NULL, "Enter world", "Game message box", MB_OK | MB_ICONINFORMATION);

    return 0;
}
