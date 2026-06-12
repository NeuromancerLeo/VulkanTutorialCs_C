#pragma once

#include <vulkan/vulkan.h>

/// @brief 该结构体定义物理设备对一个 Surface 的支持细节，以作为 选取物理设备 \ 创建交换链
/// 时的重要依据.
/// 
/// 通过调用 query_swapchain_support_details 函数来获取一个该结构体.
/// 
/// 通过调用 free_swapchain_support_details 函数来释放一个该结构体的内存.
typedef struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR    capabilities;
    VkSurfaceFormatKHR*         formats;
    VkPresentModeKHR*           presentModes;
} SwapchainSupportDetails;


/// @brief （该函数进行了 malloc，别忘了调用 free_swapchain_support_details 函数）
///
/// 查询物理设备对给定 Surface 的支持细节，并将相关信息填充至一个 `SwapchainSupportDetails`
/// 结构体并返回.
///
/// @return 填充后的 `SwapchainSupportDetails` 结构体.
SwapchainSupportDetails query_swapchain_support_details(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface    
);


/// @brief 释放结构体内按堆分配的内存.
void free_swapchain_support_details(SwapchainSupportDetails* pStructure);

/// @brief 给定物理设备查询 Surface Formats 并从中尝试选择最理想的 Surface 格式并返回.
/// 
/// @return Surface 格式 `B8G8R8A8_SRGB & SRGB_NONLINEAR_KHR`，当该格式不支持时，返回
/// 第一个可用格式
VkSurfaceFormatKHR get_optimal_surface_format(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
);


/// @brief 给定物理设备查询 Surface Present Modes 并从中尝试选择最理想的呈现模式并返回.
/// 
/// @return 呈现模式 `VK_PRESENT_MODE_MAILBOX_KHR`，当该模式不支持时，返回
/// `VK_PRESENT_MODE_FIFO_KHR` 
VkPresentModeKHR get_optimal_prensent_mode(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
);


/// @brief 查询并获取 Surface 范围（Extent）.
///
/// @param surface 给定的 Surface 句柄，用于查询其范围大小
/// @param windowFramebufferWidth 窗口宽像素大小（用于当物理设备需要我们自行设置 surface 范围
/// 时做参考）
/// @param windowFramebufferHeight 窗口高像素大小（用于当物理设备需要我们自行设置 surface 范围
/// 时做参考）
///
/// @return 创建交换链时所需要的 Surface 范围
VkExtent2D get_surface_exten(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    int                 windowFramebufferWidth,
    int                 windowFramebufferHeight
);