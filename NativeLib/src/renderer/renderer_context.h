#pragma once

#include "../common/log.h"
#include "vulkan_wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

/// @brief 渲染器上下文结构体，使用 new_renderer_context 获取一个该结构体句柄.
typedef struct RendererContext {
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

// 绘制 triangle 所需的（管线）相关对象
//（先硬编码，等我搞清楚了这一堆对象的依赖关系我再想扩展性设计的事）
    VkCommandPool       triangle_commandPool;  

    VkRenderPass        triangle_renderPass;    

    VkFramebuffer*      triangle_swapchainFramebuffers; // 堆分配的数组，需自行释放

    VkShaderModule      triangle_vertexShaderModule;
    VkShaderModule      triangle_fragmentShaderModule;
    VkPipelineLayout    triangle_pipelineLayout;
    VkPipeline          triangle_pipeline;
} RendererContext;


/// @brief 为一个渲染器上下文分配内存并返回其句柄.
///
/// @return 一个新的 RendererContext 的句柄，发生错误时返回 `NULL`
RendererContext* new_renderer_context();


/// @brief 给定一个渲染器上下文然后对其进行（初步地）初始化构建.
///
/// @param context 目标渲染器上下文句柄
/// @param window 构建渲染器上下文需要对应的窗口句柄
///
/// @return 当构建成功时返回 `true`，若发生错误则会终止构建（相关函数会输出信息）并返回 `false`
bool create_renderer_context(RendererContext* pContext, GLFWwindow* window);


/// @brief 给定渲染器上下文句柄，销毁其（除了窗口句柄外的）所有上下文对象，同时销毁自身释放内存
///
/// @param pContext 要销毁的渲染器上下文句柄
void destroy_renderer_context(RendererContext* pContext);