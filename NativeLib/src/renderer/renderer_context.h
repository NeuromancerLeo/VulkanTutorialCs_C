#pragma once

#include "../common/log.h"
#include "renderer_data_structs.h"
#include "vulkan_wrapper.h"
#include "vk_mem_alloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <pthread.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct RctxPipeline {
    VkPipeline          pipeline;
    uint32_t            vertexFirstBindingIndex;
    uint32_t            vertexBindingCount;
    VkShaderModule      vertexShaderModule;
    VkShaderModule      fragmentShaderModule;
    VkPipelineLayout    pipelineLayout;
} RctxPipeline;

typedef struct RctxMainRenderPassPipelines {
    RctxPipeline        triangle; // 属第 0 个 subpass，用于三角形绘制
    
} RctxMainRenderPassPipelines;

typedef struct RctxPipelineCreateInfo {
    const char*         vertexSpvFilePath;
    const char*         vertexSpvEntryPoint;
    const char*         fragmentSpvFilePath;
    const char*         fragmentSpvEntryPoint;
} RctxPipelineCreateInfo;

typedef struct RctxMainRenderPassPipelinesCreateInfo {
    RctxPipelineCreateInfo    triangle;
    // RctxPipelineCreateInfo    skybox
    // ......
} RctxMainRenderPassPipelinesCreateInfo;

/// @brief 渲染器上下文结构体，使用 rctxNewRendererContext 获取一个该结构体句柄.
typedef struct RendererContext {
    GLFWwindow*         window;

    VkInstance          instance;
    VkSurfaceKHR        surface;
    
    VkPhysicalDevice    physicalDevice;
    VkDevice            device;

    uint32_t            graphicsQueueFamilyIndex;
    VkQueue             graphicsQueue;

    uint32_t            presentationQueueFamilyIndex;
    VkQueue             presentationQueue;

    uint32_t            transferQueueFamilyIndex;
    VkQueue             transferQueue;    

    // 传输操作用锁，用于对应的队列和命令池，这两个 Vulkan 对象都不是线程安全的
    pthread_mutex_t     transferMutex;

    VmaAllocator        vmaAllocator;

    VkSwapchainKHR      swapchain;
    uint32_t            swapchainImageCount;
    VkImage*            swapchainImages;
    VkFormat            swapchainImageFormat;
    VkExtent2D          swapchainExtent;
    VkImageView*        swapchainImageViews;

    VkCommandPool       mainThreadCommandPool;
    VkCommandBuffer     mainCommandBuffers[MAX_FRAMES_IN_FLIGHT];

    VkCommandPool       transferCommandPool;

    VkRenderPass                    mainRenderPass;
    VkFramebuffer*                  swapchainFramebuffers; // 堆分配的数组，需自行释放
    RctxMainRenderPassPipelines     mainRenderPassPipelines;

    // 信号量：意味着一个交换链图像已被获取，其可安全渲染
    VkSemaphore         swapchainImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    // 信号量：意味着图像渲染完毕，其可安全呈现（数量为交换链图像数量）
    VkSemaphore*        renderFinishedSemaphores;
    // 栅栏：表示一个渲染进行中的帧（执行命令缓冲区），用于防止命令缓冲区发生竞态
    VkFence             frameInFlightFences[MAX_FRAMES_IN_FLIGHT];

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


/// @brief 创建并填充静态性缓冲区，内部使用 StagingBuffer 并使用传送队列复制数据到设备本地的
/// Buffer 上.
///
/// 该函数已被设计成线程安全的，你可以任意使用不同的线程来调用该函数.
/// 
/// @param usage 指示缓冲区用途
/// @param dataSize 上传数据的大小
/// @param pVerticesData 要上传的数据
///
/// @return 上传完毕后返回对应设备本地 Buffer（发生错误时句柄会设为 0）.
bool rctxCreateAndFillStaticBuffer(
    RendererContext*          pContext,
    VkBufferUsageFlagBits     usage,
    size_t                    dataSize,
    const void*               pData,
    VkBuffer*                 outBuffer,
    VmaAllocation*            outAllocation
);


/// @brief 请求销毁 Buffer 资源，该函数不会立即执行销毁，而是会将目标 Buffer 资源标记为待销毁，
/// 其会被暂存至安全的时机然后销毁，无需额外操作.
///
/// 在调用该函数后，你不应该以任何方式再次使用已被请求销毁的 Buffer 资源. 
void rctxRequestDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
);


/// @brief 立即销毁 Buffer 资源，该函数不会对目标 Buffer 资源作任何如是否被命令缓冲区占用的检查.
///
/// 一般在你确保 Buffer 资源不会被占用时才调用该函数.
void rctxDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
);


/// @brief DrawFrame 函数，请求交换链图像、检查是否重建交换链、录制主命令缓冲区、提交渲染与呈现.
///
/// @param pContext 渲染器上下文句柄
/// @param isFramebufferResized 窗口的帧缓冲区尺寸是否发生变更，该参数用于函数内部判断是否需重建
/// 交换链
/// @param pMainRenderPassPipelinesDrawInfo 主渲染通道所有管线的对应绘制信息
void rctxDrawFrame(
    RendererContext*                    pContext,
    bool                                isFramebufferResized,
    MainRenderPassPipelinesDrawInfo*    pMainRenderPassPipelinesDrawInfo
);