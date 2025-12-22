#pragma once

#include "nativelib.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

const char* requiredValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* requiredDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/// @brief 创建 VkInstance，其是程序和 Vulkan 库之间的接口.
/// 
/// @return 返回新创建的 VkInstance 句柄（当发生错误时返回 `NULL`）
EX_API VkInstance createInstance(void);

/// @brief 查询对 VkInstance 可用的层并打印出来，并检查请求的层是否可用.
///
/// @return 当检查到有请求的层不可用时，该函数会打印相关信息，并返回 `false`
bool check_instance_layer_properties(void);

/// @brief 查询对 Vulkan Instance 可用的扩展并打印出来.
void check_instance_extension_properties(void);


/// @brief 销毁给定的 VkInstance.
EX_API void destroyInstance(VkInstance instance);


/// @brief 在成功创建 VkInstance 后调用该函数创建 VkSurfaceKHR，其用于将渲染的图形呈现到屏幕上.
/// 
/// @param instance 调用该函数需要传入一个 VkInstance 句柄
/// （需确保已启用了 `VK_KHR_surface` 扩展和平台相关的扩展如 `VK_KHR_win32_surface`）
/// @param window 调用该函数需要传入一个 GLFWwindow 句柄
///
/// @return 返回新创建的 VkSurfaceKHR 句柄（当发生错误时返回 `NULL`）
EX_API VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);


/// @brief 销毁给定的 VkSurfaceKHR.
///
/// @param instance 调用该函数需要传入一个 VkInstance 句柄
EX_API void destroySurface(VkInstance instance, VkSurfaceKHR surface);


/// @brief 查询可用物理设备并尝试选择可用的显卡作 PhysicalDevice.
///
/// @param instance 调用该函数需要传入一个 VkInstance 句柄
/// @param surface 调用该函数需要传入一个 VkSurfaceKHR 句柄
///
/// @return 返回一个可用的 PhysicalDevice 句柄（当发生错误时返回 `NULL`）
EX_API VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

/// @brief 该函数用于检查传入的物理设备的某个属性/支持功能是否符合要求.
///
/// （至于具体要求详见函数）
///
/// @return `true` 当物理设备符合所有要求时，反之返回 `false`
bool is_physical_device_suitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

/// @brief 查询给定物理设备可用的扩展并打印出来，并检查请求的扩展是否可用
///
/// @return 当检查到有请求的扩展不可用时，该函数会打印相关信息，并返回 `false`
bool check_device_extension_properties(VkPhysicalDevice physicalDevice);

void dump_physical_device_properties(VkPhysicalDevice physicalDevice);


/// @brief 根据给定物理设备创建逻辑设备.
///
/// @param graphicsQueue 函数执行成功后，该参数会接收一个新的 VkQueue 句柄（graphics）
/// @param presentationQueue 函数执行成功后，该参数会接收一个新的 VkQueue 句柄（presentation）
///
/// @return 返回新创建的 VkDevice 句柄（当发生错误时返回 `NULL`）
EX_API VkDevice createLogicalDevice(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    VkQueue*            graphicsQueue,
    VkQueue*            presentationQueue
);


/// @brief 销毁给定的 VkDevice.
EX_API void destroyLogicalDevice(VkDevice device);


/// @brief 根据给定窗口句柄和设备创建交换链.
///
/// @param window 给定窗口句柄
/// @param surface 给定 Surface 句柄
/// @param physicalDevice 给定物理设备句柄
/// @param device 给定设备句柄
///
/// @return 返回新创建的 VkSwapchainKHR 句柄（当发生错误时返回 `NULL`）
EX_API VkSwapchainKHR createSwapchain(
    GLFWwindow*         window,
    VkSurfaceKHR        surface,
    VkPhysicalDevice    physicalDevice, 
    VkDevice            device
);


/// @brief 销毁给定的 VkSwapchainKHR.
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
EX_API void destroySwapchain(VkDevice device, VkSwapchainKHR swapchain);