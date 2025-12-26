#pragma once

#include "../common/ansi_esc.h"
#include "queue_family_indices.h"
#include "swapchain_support_details.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


/// @brief 创建 VkInstance，其是程序和 Vulkan 库之间的接口.
/// 
/// @return 返回新创建的 VkInstance 句柄（当发生错误时返回 `NULL`）
VkInstance createInstance(void);


/// @brief 销毁给定的 VkInstance.
void destroyInstance(VkInstance instance);


/// @brief 在成功创建 VkInstance 后调用该函数创建 VkSurfaceKHR，其是对窗口系统中
/// 具体窗口对象 的抽象.
///
/// @param instance 调用该函数需要传入一个有效的 VkInstance 句柄
/// （需确保已启用了 `VK_KHR_surface` 扩展和平台相关的扩展如 `VK_KHR_win32_surface`）
/// @param window 调用该函数需要传入一个有效的 GLFWwindow 句柄
///
/// @return 返回新创建的 VkSurfaceKHR 句柄（当发生错误时返回 `NULL`）
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);


/// @brief 销毁给定的 VkSurfaceKHR.
///
/// @param instance 调用该函数需要传入一个对应的 VkInstance 句柄
void destroySurface(VkInstance instance, VkSurfaceKHR surface);


/// @brief 查询可用物理设备并尝试选择可用的显卡作 PhysicalDevice.
///
/// @param instance 调用该函数需要传入一个有效的 VkInstance 句柄
/// @param surface 调用该函数需要传入一个有效的 VkSurfaceKHR 句柄
///
/// @return 返回一个可用的 PhysicalDevice 句柄（当发生错误时返回 `NULL`）
VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);


/// @brief 根据给定物理设备创建逻辑设备.
///
/// @param graphicsQueue 函数执行成功后，该参数会接收一个新的 VkQueue 句柄（graphics）
/// @param presentationQueue 函数执行成功后，该参数会接收一个新的 VkQueue 句柄（presentation）
///
/// @return 返回新创建的 VkDevice 句柄（当发生错误时返回 `NULL`）
VkDevice createLogicalDevice(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    VkQueue*            graphicsQueue,
    VkQueue*            presentationQueue
);


/// @brief 销毁给定的 VkDevice.
void destroyLogicalDevice(VkDevice device);


/// @brief 根据给定窗口句柄和设备创建交换链.
///
/// @param window 给定窗口句柄
/// @param surface 给定 Surface 句柄
/// @param physicalDevice 给定物理设备句柄
/// @param device 给定设备句柄
/// @param pSwapchainImageCount 输出参数，交换链创建后其输出交换链图像句柄数组的大小
/// @param ppSwapchainImages 输出参数，其输出一个指向交换链图像句柄数组的指针
///
/// @return 返回新创建的 VkSwapchainKHR 句柄（当发生错误时返回 `NULL`）
VkSwapchainKHR createSwapchain(
    GLFWwindow*         window,
    VkSurfaceKHR        surface,
    VkPhysicalDevice    physicalDevice, 
    VkDevice            device,
    uint32_t*           pSwapchainImageCount,
    VkImage**           ppSwapchainImages,
    VkFormat*           pSwapchainImageFormat,
    VkExtent2D*         pSwapchainExtent    
);


/// @brief 销毁给定的 VkSwapchainKHR.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param swapchain 要销毁的交换链句柄
void destroySwapchain(VkDevice device, VkSwapchainKHR swapchain);


/// @brief 成功创建交换链后调用该函数为交换链中的每一个图像创建基本的图像视图（VkImageView）.
///
/// @param device 调用该函数需要传入对应的 VkDevice 句柄
/// @param swapchainImageFormat 指定要创建的图像视图的图像格式
/// @param swapchainImageCount 指定要创建的图像视图的数量
/// @param pSwapchainImages 调用该函数需要传入对应的交换链图像数组
///
/// @return 创建成功后返回一个属于交换链的图像视图数组，失败则返回 `NULL`
VkImageView* createSwapchainImageViews(
    VkDevice        device,
    VkFormat        swapchainImageFormat,
    uint32_t        swapchainImageCount,
    const VkImage*  pSwapchainImages
);


/// @brief 销毁交换链所有的图像视图.
///
/// @param device 调用该函数需要传入对应的 VkDevice 句柄
/// @param swapchainImageCount 交换链图像总数
/// @param ppSwapchainImageViews 要销毁的交换链图像视图的数组的地址
///（数组本身也会被销毁以释放内存）
void destroySwapchainImageViews(
    VkDevice        device,
    uint32_t        swapchainImageCount,
    VkImageView**   ppSwapchainImageViews
);