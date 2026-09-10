#pragma once

#include <stdio.h>


#ifdef _WIN32

#define DEBUG_BREAK() __debugbreak()

#endif


namespace
{

    enum TextColor
    {
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
        printf("%s %s\n", prefix, msg);
#else
        printf("%s %s %s \033[0m\n", TextColorCodes[text_color], prefix, msg);
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
        char format_buffer[8192] = {};
        sprintf(format_buffer, msg, args...);

#ifdef LOG_COLOR_OFF
        printf("%s %s\n", prefix, format_buffer);
#else
        printf("%s %s %s\n \033[0m\n", TextColorCodes[text_color], prefix, format_buffer);
#endif
    }


}; // namespace


#define LOG_INFO(msg) _log(TEXT_COLOR_BOLD_WHITE, "[INFO]:", msg);
#define LOG_OK(msg) _log(TEXT_COLOR_BOLD_GREEN, "[OK]:", msg);
#define LOG_WARN(msg) _log(TEXT_COLOR_BOLD_YELLOW, "[WARN]:", msg);
#define LOG_ERROR(msg) _log(TEXT_COLOR_BOLD_RED, "[ERROR]:", msg);

#define LOG_INFOF(msg, ...) _logf(TEXT_COLOR_BOLD_WHITE, "[INFO]:", msg, ##__VA_ARGS__);
#define LOG_OKF(msg, ...) _logf(TEXT_COLOR_BOLD_GREEN, "[OK]:", msg, ##__VA_ARGS__);
#define LOG_WARNF(msg, ...) _logf(TEXT_COLOR_BOLD_RED, "[WARN]:", msg, ##__VA_ARGS__);
#define LOG_ERRORF(msg, ...) _logf(TEXT_COLOR_BOLD_RED, "[ERROR]:", msg, ##__VA_ARGS__);


#define LOG_ASSERT(condition, msg, ...)     \
    {                                       \
        if (!(condition))                   \
        {                                   \
            LOG_ERRORF(msg, ##__VA_ARGS__); \
            DEBUG_BREAK();                  \
        }                                   \
    }
