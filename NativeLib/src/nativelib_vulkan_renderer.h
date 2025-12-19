#pragma once

#include <vulkan/vulkan.h>
#include <stdbool.h>

/// @brief 该结构体定义物理设备可能拥有的队列族类型及其索引.
///
/// (索引值为 -1 表未找到)
typedef struct QueueFamilyIndices
{
    int graphicsFamily;     // 图形用
}QueueFamilyIndices;

/// @brief 查询给定物理设备所拥有的队列族，并按其 `queueFlags` 填充 `QueueFamilyIndices`
///
/// 结构体中的索引字段以返回.
/// @param physicalDevice 给定的物理设备
/// @return 填充后的 `QueueFamilyIndices` 结构体
QueueFamilyIndices find_queue_families(VkPhysicalDevice physicalDevice)
{
    QueueFamilyIndices queueFamilyIndices = 
    {
        .graphicsFamily = -1    // 初始化，-1 表未找到
    };

    // 1.查询队列族 Properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount, 
        NULL);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount,
        queueFamilies);
    
    // 2.遍历队列族 Properties
    for (int j = 0; j < queueFamilyCount; j++)
    {
        // 检查其队列 flags
        if(queueFamilies[j].queueCount > 0)
        {
            if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queueFamilyIndices.graphicsFamily += 1;
        }
    }

    return queueFamilyIndices;

}