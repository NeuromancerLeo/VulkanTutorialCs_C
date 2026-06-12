/// 该 .cpp 文件用于 VMA 的具体实现，如需使用 VMA，包含 vk_mem_alloc.h 头文件即可.
#include "../../../common/log.h"

#define VMA_VULKAN_VERSION 1003000 // 针对 Vulkan 1.3 编译

#ifdef DEBUG
    // 定义 VMA 使用的日志打印函数宏
    #define VMA_DEBUG_LOG(format, ...) log_trace(ESC_BCOLOR_BRIGHT_MAGENTA "VMA: " format ESC_RESET, ##__VA_ARGS__)
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"