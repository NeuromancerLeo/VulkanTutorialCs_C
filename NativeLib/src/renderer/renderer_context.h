#pragma once

#include "../common/log.h"
#include "renderer_data_structs.h"
#include "vulkan_wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct RctxPipeline {
    VkPipeline          pipeline;
    VkShaderModule      vertexShaderModule;
    VkShaderModule      fragmentShaderModule;
    VkPipelineLayout    pipelineLayout;
} RctxPipeline;

/// @brief 渲染器上下文结构体，使用 rctxNewRendererContext 获取一个该结构体句柄.
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

    VkCommandPool       mainThreadCommandPool;   // TODO: mainThreadCommandPool
    VkCommandBuffer     mainCommandBuffers[MAX_FRAMES_IN_FLIGHT];  // TODO: mainCommandBuffers

    VkRenderPass        mainRenderPass;    // TODO: mainRenderPass
    VkFramebuffer*      swapchainFramebuffers; // 堆分配的数组，需自行释放 // TODO: swapchainFramebuffers
    RctxPipeline        mainRenderPassPipeline0;
    // VkShaderModule      triangle_vertexShaderModule;
    // VkShaderModule      triangle_fragmentShaderModule;
    // VkPipelineLayout    triangle_pipelineLayout;
    // VkPipeline          triangle_pipeline;  // TODO: mainRenderPassPipeline0

    // 信号量：意味着一个交换链图像已被获取，其可安全渲染
    VkSemaphore         swapchainImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    // 信号量：意味着图像渲染完毕，其可安全呈现（数量为交换链图像数量）
    VkSemaphore*        renderFinishedSemaphores;
    // 栅栏：表示一个渲染进行中的帧（执行命令缓冲区），用于防止命令缓冲区发生竞态
    VkFence             frameInFlightFences[MAX_FRAMES_IN_FLIGHT];

    // 仅临时
    VkBuffer            triangle_vertexBuffer;
    VkDeviceMemory      triangle_vertexBufferMemory;
} RendererContext;


/// @brief 为一个渲染器上下文分配内存并返回其句柄.
///
/// @return 一个新的 RendererContext 的句柄，发生错误时返回 `NULL`
RendererContext* rctxNewRendererContext();


/// @brief 给定一个渲染器上下文然后对其进行（初步地）初始化构建.
///
/// @param context 目标渲染器上下文句柄
/// @param window 构建渲染器上下文需要对应的窗口句柄
///
/// @return 当构建成功时返回 `true`，若发生错误则会终止构建（相关函数会输出信息）并返回 `false`
bool rctxCreateRendererContext(RendererContext* pContext, GLFWwindow* window);


/// @brief 给定渲染器上下文句柄，销毁其（除了窗口句柄外的）所有上下文对象，同时销毁自身释放内存
///
/// @param pContext 要销毁的渲染器上下文句柄
void rctxDestroyRendererContext(RendererContext* pContext);


bool triangle_allocate_and_fill_vertex_buffer(
    RendererContext*    pContext,
    size_t              dataSize,
    const VertexData*   pVertexData
);


/// @brief DrawFrame 函数，请求交换链图像、检查是否重建交换链、录制主命令缓冲区、提交渲染与呈现.
///
/// @param pContext 渲染器上下文句柄
/// @param isFramebufferResized 窗口的帧缓冲区尺寸是否发生变更，该参数用于函数内部判断是否需重建
/// 交换链
void rctxDrawFrame(
    RendererContext*                    pContext,
    bool                                isFramebufferResized,
    MainRenderPassPipeline0DrawInfo*    pMainRenderPassPipeline0DrawInfo
);