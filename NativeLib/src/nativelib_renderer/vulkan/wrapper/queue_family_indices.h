#pragma once

#include "../../../common/log.h"

#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <string.h>

/// @brief 该结构体定义物理设备拥有的队列族类型及其索引, 以作为 选取物理设备 \ 创建队列
/// 时的重要依据.
///
/// 通过调用 find_queue_families 函数来查询以填充该结构体.
///
/// (索引值为 -1 表未找到)
typedef struct QueueFamilyIndices {
    int graphicsSupport;        // 支持 图形
    int presentationSupport;    // 支持 呈现
    int transferSupport;        // 支持 传输
} QueueFamilyIndices;


/// @brief 查询给定物理设备所拥有的队列族，并按其队列支持相关信息填充传入的
/// `QueueFamilyIndices` 结构体中的索引字段.
///
/// 该函数会考虑并填充所有的索引字段.
///
/// @param physicalDevice 给定的物理设备
/// @param surface 给定的窗口表面
/// @param pFamilyIndices 输出参数，查询完成后结构体内对应字段会被设置为可用的队列族索引
void find_queue_families(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    QueueFamilyIndices* pFamilyIndices
);

/// @brief 查询给定物理设备是否有队列族同时支持图形命令和（给定 Surface 的）呈现.
///
/// @param queueFamilyIndex 查询到有队列族满足条件后会将其索引赋值给该指针变量供调用者使用（反
/// 之则会被赋值为 -1）(若不需要该参数传入 `NULL` 即可)
///
/// @return `true` 当查询到有队列族满足条件，反之为 `false`
bool has_queue_family_supports_both_graphics_and_presentation(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    int*                queueFamilyIndex
);
