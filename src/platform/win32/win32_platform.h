#pragma once

#include "src/pch.h"

#include "src/utils/game_math.h"

// =================================================================================
//                      PLATFORM SERVICES STRUCTS DEFINITIONS
// =================================================================================
struct PlatformWindow
{
    HWND Handle;
    bool IsFullscreen;
    // win rect client-area in pixels
    Vec2 PrevClientRectSize;
    // win rect client-area in pixels
    Vec2 ClientRectSize;
    // in screen coordinates
    Vec2 Pos;
};
