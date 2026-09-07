#pragma once

#include <stdio.h>


#ifdef _WIN32

#include <Windows.h>
#define DEBUG_BREAK() __debugbreak()
// #define LOG_COLOR_OFF

#endif


namespace
{

    enum TextColor
    {
        TEXT_COLOR_GRAY,
        TEXT_COLOR_WHITE,
        TEXT_COLOR_RED,
        TEXT_COLOR_YELLOW,
        TEXT_COLOR_GREEN,
        TEXT_COLOR_BOLD_WHITE,
        TEXT_COLOR_BOLD_RED,
        TEXT_COLOR_BOLD_YELLOW,
        TEXT_COLOR_BOLD_GREEN,
        TEXT_COLOR_COUNT,
    };
    static const char* TextColorCodes[TEXT_COLOR_COUNT] = {
        /* TEXT_COLOR_GRAY        */ "\x1b[38;5;250m",
        /* TEXT_COLOR_WHITE       */ "\x1b[38;5;255m",
        /* TEXT_COLOR_RED         */ "\x1b[38;5;210m",
        /* TEXT_COLOR_YELLOW      */ "\x1b[38;5;229m",
        /* TEXT_COLOR_GREEN       */ "\x1b[38;5;157m",
        /* TEXT_COLOR_BOLD_WHITE  */ "\x1b[1;38;5;255m",
        /* TEXT_COLOR_BOLD_RED    */ "\x1b[1;38;5;210m",
        /* TEXT_COLOR_BOLD_YELLOW */ "\x1b[1;38;5;229m",
        /* TEXT_COLOR_BOLD_GREEN  */ "\x1b[1;38;5;157m",
    };


    /*
        Prints a debug message to the windows console using OutputDebugString.

        NOTE(harsh): This function is not intended to be used standalone, it should
        be used with LogASSERT_ functions like so: LogASSERTError();
    */
    inline void _log(TextColor text_color, const char* prefix, const char* msg)
    {
#ifdef LOG_COLOR_OFF
        char text_buffer[8192] = {};
        sprintf(text_buffer, "%s %s", prefix, msg);
#else
        char text_buffer[8192] = {};
        sprintf(text_buffer, "%s %s %s \033[0m", TextColorCodes[text_color], prefix, msg);
#endif

#ifdef _WIN32
        OutputDebugString(text_buffer);
        OutputDebugString("\n");
#endif
    }

    /*
        Prints a debug message to the windows console using OutputDebugString.

        NOTE(harsh): This function is not intended to be used standalone, it should
        be used with LOG_LEVEL functions, for e.g. LOG_ERROR_F("some error info %d", error_code);
    */
    template <typename... Args>
    inline void _logf(TextColor text_color, const char* prefix, const char* msg, Args... args)
    {
#ifdef LOG_COLOR_OFF
        char format_buffer[8192] = {};
        sprintf(format_buffer, "%s %s", prefix, msg);
#else
        char format_buffer[8192] = {};
        sprintf(format_buffer, "%s %s %s \033[0m", TextColorCodes[text_color], prefix, msg);
#endif

        char text_buffer[8192] = {};
        sprintf(text_buffer, format_buffer, args...);

#ifdef _WIN32
        OutputDebugString(text_buffer);
        OutputDebugString("\n");
        puts(text_buffer);
#endif
    }


}; // namespace


#define LOG_ERROR(msg) _log(TEXT_COLOR_BOLD_RED, "[ERROR]:", msg);
#define LOG_ERRORF(msg, ...) _logf(TEXT_COLOR_BOLD_RED, "[ERROR]:", msg, ##__VA_ARGS__);


// #define LOG_ASSERT(condition, msg, ...)    \
//     {                                      \
//         if (!(condition))                  \
//         {                                  \
//             LOG_ERROR(msg, ##__VA_ARGS__); \
//             DEBUG_BREAK();                 \
//         }                                  \
//     }
