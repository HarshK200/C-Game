#pragma once


enum INPUT_ACTION
{
    ACTION_MOVE_UP = 0,
    ACTION_MOVE_DOWN = 1,
    ACTION_MOVE_LEFT = 2,
    ACTION_MOVE_RIGHT = 3,
    INPUT_ACTION_COUNT,
};

enum ACTION_STATE
{
    IDLE = 0,
    PRESSED = 1,
    HELD = 2,
    RELEASED = 3,
    ACTION_STATE_COUNT,
};

struct InputManager
{
    ACTION_STATE ActionMap[INPUT_ACTION_COUNT];
    bool WindowResized = false;

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
