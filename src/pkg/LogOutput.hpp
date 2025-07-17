#pragma once

#ifndef _BECAM_LOG_OUTPUT_H_
#define _BECAM_LOG_OUTPUT_H_

#include <iostream>
#include <sstream>

#if defined(_WIN32)
    #include <windows.h>
    #define DEBUG_OUTPUT(str) OutputDebugStringA(str)
#else
    #define DEBUG_OUTPUT(str) ((void)0)  // no-op for non-Windows
#endif

#define DEBUG_LOG(...)                                                              \
    do {                                                                            \
        std::ostringstream oss;                                                     \
        oss << "[DEBUG] " << __VA_ARGS__ << std::endl;                              \
        std::string logStr = oss.str();                                             \
        std::cerr << logStr;                                                        \
        DEBUG_OUTPUT(logStr.c_str());                                               \
    } while (0)

#endif
