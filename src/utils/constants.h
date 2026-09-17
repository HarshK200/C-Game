#pragma once

#include "game_math.h"


// constants
inline constexpr double FIXED_PHYSICS_DELTA_TIME = 0.01f; // in seconds i.e. 10ms per physics update
inline constexpr Vec2 INTERNAL_RENDER_RESOLUTION = {640.0f, 360.0f};
inline constexpr Vec2 DEFAULT_WINDOW_RESOLUTION = {1280.0f, 720.0f};
inline constexpr int unsigned MAX_RENDER_COMMANDS_PER_FRAME = 100000;
inline constexpr int unsigned MAX_INSTANCE_BUFFER_SIZE = 500;
