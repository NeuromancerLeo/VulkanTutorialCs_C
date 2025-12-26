#pragma once

#include "../common/ansi_esc.h"
#include "vulkan_wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

/// @brief 渲染上下文结构体，使用 new_render_context 获取一个该结构体句柄.
typedef struct RenderContext {
    GLFWwindow*         window;

    VkInstance          instance;
    VkSurfaceKHR        surface;
    
    VkPhysicalDevice    physicalDevice;
    VkDevice            device;
    VkQueue             graphicsQueue;
    VkQueue             presentationQueue;

    VkSwapchainKHR      swapchain;
    uint32_t            swapchainImageCount;
    VkImage*            swapchainImages;
    VkFormat            swapchainImageFormat;
    VkExtent2D          swapchainExtent;
    VkImageView*        swapchainImageViews;
} RenderContext;


/// @brief 为一个渲染上下文分配内存并返回其句柄.
///
/// @return 一个新的 RenderContext 的句柄，发生错误时返回 `NULL`
RenderContext* new_render_context();

/// @brief 给定一个渲染上下文然后对其进行初始化构建.
///
/// @param window 构建渲染上下文需要对应的窗口句柄
/// @param context 目标渲染上下文句柄
///
/// @return 当构建成功时返回 `true`，若发生错误则会终止构建（相关函数会输出信息）并返回 `false`
bool create_render_context(GLFWwindow* window, RenderContext* pContext);

/// @brief 给定渲染上下文句柄，销毁其（除了窗口句柄外的）所有上下文对象，同时销毁自身释放内存
/// @param pContext 要销毁的渲染上下文句柄
void destroy_render_context(RenderContext* pContext);