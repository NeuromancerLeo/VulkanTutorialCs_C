#pragma once

#include <vulkan/vulkan.h>
#include <stdbool.h>

/// @brief 该结构体定义物理设备可能拥有的队列族类型及其索引.
///
/// (索引值为 -1 表未找到)
typedef struct QueueFamilyIndices
{
    int graphicsSupport;        // 支持 图形
    int presentationSupport;    // 支持 呈现
}QueueFamilyIndices;

QueueFamilyIndices init_QueueFamilyIndices()
{
    // 初始化，-1 表未找到
    QueueFamilyIndices queueFamilyIndices = 
    {
        .graphicsSupport        = -1,   
        .presentationSupport    = -1
    };

    return queueFamilyIndices;
}

/// @brief 查询给定物理设备所拥有的队列族，并按其队列支持相关信息填充 `QueueFamilyIndices`
/// 结构体中的索引字段以返回.
///
/// 该函数会考虑并填充所有的索引字段.
///
/// @param physicalDevice 给定的物理设备
///
/// @return 填充后的 `QueueFamilyIndices` 结构体
QueueFamilyIndices find_queue_families(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
)
{
    QueueFamilyIndices queueFamilyIndices = init_QueueFamilyIndices();

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
    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueCount < 1)
            continue;
        
        // 检查其队列 flags
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsSupport = i;

        // 检查其是否支持呈现
        VkBool32 supportsPresentation = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
            i,
            surface,
            &supportsPresentation);
        if (supportsPresentation == VK_TRUE)
            queueFamilyIndices.presentationSupport = i;
    }

    return queueFamilyIndices;

}

/// @brief 查询给定物理设备是否有队列族同时支持图形命令和（给定 Surface 的）呈现.
///
/// @param queueFamilyIndex 查询到有队列族满足条件后会将其索引赋值给该指针变量供调用者使用（反
/// 之则会被赋值为 -1）
///
/// @return `true` 当查询到有队列族满足条件，反之为 `false`
bool has_queue_family_supports_both_graphics_and_presentation(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    int*                queueFamilyIndex
)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount, 
        NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount,
        queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueCount < 1)    // 我想不通有什么b显卡有队列族没队列的
            continue;

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 supportsPresentation = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                i,
                surface,
                &supportsPresentation);
            
            // 找到符合条件的马上设置传入队列族索引并返回 true
            if (supportsPresentation == VK_TRUE)
            {
                *queueFamilyIndex = i;
                return true;
            }
        }
    }

    *queueFamilyIndex = -1;     // 设为 -1 表未找到
    return false;
}