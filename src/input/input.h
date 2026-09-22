#pragma once


enum INPUT_ACTION
{
    // debug actions
    ACTION_ZOOM_IN_DEBUG = 0,
    ACTION_ZOOM_OUT_DEBUG = 1,
    ACTION_ZOOM_RESET_DEBUG = 2,

    // movement actions
    ACTION_MOVE_UP = 3,
    ACTION_MOVE_DOWN = 4,
    ACTION_MOVE_LEFT = 5,
    ACTION_MOVE_RIGHT = 6,

    // window actions
    ACTION_WINDOWED_FULLSCREEN = 7,
    ACTION_WINDOW_RESIZE = 8,
    ACTION_WINDOW_MOVE = 9,

    INPUT_ACTION_COUNT,
};

enum ACTION_STATE
{
    IDLE = 0,
    PRESSED = 1,
    HELD = 2,
    RELEASED = 3,
    SINGLE_PRESSED = 4,
    ACTION_STATE_COUNT,
};

struct InputManager
{
    ACTION_STATE ActionMap[INPUT_ACTION_COUNT];

    inline bool IsActionSinglePressed(INPUT_ACTION action)
    {
        return (this->ActionMap[action] == SINGLE_PRESSED);
    }
    inline bool IsActionPressed(INPUT_ACTION action)
    {
        return (this->ActionMap[action] == PRESSED);
    }
    inline bool IsActionHeld(INPUT_ACTION action)
    {
        return (this->ActionMap[action] == HELD);
    }
    inline bool IsActionReleased(INPUT_ACTION action)
    {
        return (this->ActionMap[action] == RELEASED);
    }
};
