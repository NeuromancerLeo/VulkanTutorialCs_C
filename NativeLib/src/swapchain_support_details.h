#pragma once

#include "ansi_esc.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

/// @brief 该结构体定义物理设备对一个 Surface 的支持细节，以作为 选取物理设备 \ 创建交换链
/// 时的重要依据.
/// 
/// 通过调用 query_swapchain_support_details 函数来获取一个该结构体.
/// 
/// 通过调用 free_swapchain_support_details 函数来释放一个该结构体的内存.
typedef struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR    capabilities;
    VkSurfaceFormatKHR*         formats;
    VkPresentModeKHR*           presentModes;
}SwapchainSupportDetails;

/// @brief （该函数进行了 malloc，别忘了调用 free_swapchain_support_details 函数）
///
/// 查询物理设备对给定 Surface 的支持细节，并将相关信息填充至一个 `SwapchainSupportDetails`
/// 结构体并返回.
///
/// @return 填充后的 `SwapchainSupportDetails` 结构体.
SwapchainSupportDetails query_swapchain_support_details(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface    
)
{
    // 0.初始化
    SwapchainSupportDetails supportDetails = 
    {
        .formats = NULL,
        .presentModes = NULL
    };

    // 1.VkSurfaceCapabilitiesKHR
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, 
        surface, 
        &supportDetails.capabilities);

    // 2.VkSurfaceFormatKHR
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

    if (formatCount)
    {
        supportDetails.formats = (VkSurfaceFormatKHR*)
            malloc(formatCount * sizeof(VkSurfaceFormatKHR));

        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, 
            surface, 
            &formatCount, 
            supportDetails.formats);
    }
    else
    {
        fprintf(stderr, 
            ESC_FCOLOR_BRIGHT_RED 
            "No avaliable surface format could be found!\n" ESC_RESET);

        supportDetails.formats = NULL;
    }

    // 3.VkPresentModeKHR
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface, 
        &presentModeCount,
        NULL);

    if (presentModeCount)
    {
        supportDetails.presentModes = (VkPresentModeKHR*)
            malloc(presentModeCount * sizeof(VkPresentModeKHR));

        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface, 
        &presentModeCount,
        supportDetails.presentModes);
    }
    else
    {
        fprintf(stderr, 
            ESC_FCOLOR_BRIGHT_RED 
            "No avaliable surface present mode could be found!\n" ESC_RESET);

        supportDetails.presentModes = NULL;
    }

    return supportDetails;
}

/// @brief 释放结构体内按堆分配的内存.
void free_swapchain_support_details(SwapchainSupportDetails* pStructure)
{
    if(pStructure == NULL)
        return;

    if (pStructure->formats != NULL)
    {
        free(pStructure->formats);
        pStructure->formats = NULL;
    }

    if (pStructure->presentModes != NULL)
    {
        free(pStructure->presentModes);
        pStructure->presentModes = NULL;
    }
}