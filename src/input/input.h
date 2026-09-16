#pragma once


enum INPUT_ACTION
{
    MOVE_UP = 0,
    MOVE_DOWN = 1,
    MOVE_LEFT = 2,
    MOVE_RIGHT = 3,
    INPUT_ACTION_COUNT,
};

enum ACTION_STATE
{
    PRESSED = 0,
    HELD = 1,
    RELEASED = 2,
    ACTION_STATE_COUNT,
};

struct InputManager
{
    ACTION_STATE ActionMap[INPUT_ACTION_COUNT];
    bool WindowResized = false;
};
