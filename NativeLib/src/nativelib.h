#pragma once

#ifdef _WIN64       // _WIN32 由 Windows 平台上的编译器自动定义
    #define EX_API __declspec(dllexport)
#endif

#ifdef __linux__    // __linux__ 由 Linux 平台上的编译器自动定义
    #define EX_API __attribute__((visibility("default")))
#endif