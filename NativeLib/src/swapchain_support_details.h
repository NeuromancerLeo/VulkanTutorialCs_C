#pragma once

#include "ansi_esc.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
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

/// @brief 给定物理设备查询 Surface Formats 并从中尝试选择最理想的 Surface 格式并返回.
/// 
/// @return Surface 格式 `B8G8R8A8_SRGB & SRGB_NONLINEAR_KHR`，当该格式不支持时，返回
/// 第一个可用格式
VkSurfaceFormatKHR get_optimal_surface_format(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
)
{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

    VkSurfaceFormatKHR surfaceFormats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, 
        surface, 
        &formatCount, 
        surfaceFormats);
    
    for (int i = 0; i < formatCount; i++)
    {
        // 理想选择为 B8G8R8A8_SRGB 和 SRGB_NONLINEAR_KHR
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormats[i];
        }
    }

    fprintf(stdout, 
        "物理设备不支持表面格式 B8G8R8A8_SRGB & SRGB_NONLINEAR_KHR，"
        "将使用第一个可用的表面格式！\n");

    return surfaceFormats[0];
}

/// @brief 给定物理设备查询 Surface Present Modes 并从中尝试选择最理想的呈现模式并返回.
/// 
/// @return 呈现模式 `VK_PRESENT_MODE_MAILBOX_KHR`，当该模式不支持时，返回
/// `VK_PRESENT_MODE_FIFO_KHR` 
VkPresentModeKHR get_optimal_prensent_mode(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
)
{
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface,
        &presentModeCount,
        NULL);

    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface,
        &presentModeCount,
        presentModes);

    for (int i = 0; i < presentModeCount; i++)
    {
        // 理想选择为 VK_PRESENT_MODE_MAILBOX_KHR
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    fprintf(stdout, 
        "物理设备不支持 MAILBOX 呈现模式，"
        "将使用 FIFO 呈现模式！\n");

    return VK_PRESENT_MODE_FIFO_KHR;
}

static inline int clamp_int(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/// @brief 查询并获取交换范围（Swap Extent）.
///
/// @param surface 给定的 Surface 句柄，用于查询交换范围大小
/// @param window 给定的 GLFWwindow 句柄，用于获取窗口像素大小
/// （当窗口管理器需要我们自行设置交换范围时）
///
/// @return 创建交换链时所需要的交换范围
VkExtent2D get_swap_exten(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    GLFWwindow*         window
)
{
    VkSurfaceCapabilitiesKHR capabilities = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, 
        surface, 
        &capabilities);

    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    // 若窗口管理器需要我们自行设置交换范围（ ^ 即高或宽是 uint32_t 的最大值）

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    width = clamp_int(width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width);

    height = clamp_int(height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);

    VkExtent2D actualExtent = {
        .width = width,
        .height = height
    };

    return actualExtent;
}