#pragma once

#include "../../vulkan/vma/vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
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
    RctxPipeline        unlit;    // 属第 0 个 subpass，用于 3D 图形绘制
} RctxMainRenderPassPipelines;

typedef struct RctxPipelineCreateInfo {
    const char*         vertexSpvFilePath;
    const char*         vertexSpvEntryPoint;
    const char*         fragmentSpvFilePath;
    const char*         fragmentSpvEntryPoint;
} RctxPipelineCreateInfo;

typedef struct RctxMainRenderPassPipelinesCreateInfo {
    RctxPipelineCreateInfo    triangle;
    RctxPipelineCreateInfo    unlit;
    // RctxPipelineCreateInfo    skybox
    // ......
} RctxMainRenderPassPipelinesCreateInfo;

/// @brief 渲染器上下文所定义的管线的枚举.
typedef enum RctxPipelineType {
    RCTX_PIPELINE_TYPE_TRIANGLE,
    RCTX_PIPELINE_TYPE_UNLIT
    // ...
} RctxPipelineType;

/// @brief 渲染器上下文结构体，使用 rctxNewRendererContext 获取一个该结构体句柄.
typedef struct RendererContext {
    uint32_t                    currentFrameInFlightIndex;

    GLFWwindow*                 window;

    VkInstance                  instance;
    VkSurfaceKHR                surface;
    
    VkPhysicalDevice            physicalDevice;
    VkPhysicalDeviceProperties  physicalDeviceProperties;

    VkDevice                    device;
    uint32_t                    graphicsQueueFamilyIndex;
    VkQueue                     graphicsQueue;
    uint32_t                    presentationQueueFamilyIndex;
    VkQueue                     presentationQueue;
    uint32_t                    transferQueueFamilyIndex;
    VkQueue                     transferQueue;    
    // 传输操作用锁，用于对应的队列和命令池，这两个 Vulkan 对象都不是线程安全的
    pthread_mutex_t             transferMutex;

    VmaAllocator                vmaAllocator;

    VkSwapchainKHR              swapchain;
    uint32_t                    swapchainImageCount;
    VkImage*                    swapchainImages;
    VkFormat                    swapchainImageFormat;
    VkExtent2D                  swapchainExtent;
    VkImageView*                swapchainImageViews;

    VkCommandPool               mainThreadCommandPool;
    VkCommandBuffer             mainCommandBuffers[MAX_FRAMES_IN_FLIGHT];

    VkCommandPool               transferCommandPool;

    VkRenderPass                    mainRenderPass;
    VkFramebuffer*                  swapchainFramebuffers; // 堆分配的数组，需自行释放
    
    VkDescriptorSetLayout           cameraDescriptorSetLayout;
    VkDescriptorPool                cameraDescriptorSetPool;
    VkDescriptorSetLayout           drawItemsDescriptorSetLayout;
    VkDescriptorPool                drawItemsDescriptorSetPool;

    RctxMainRenderPassPipelines     mainRenderPassPipelines;

    // 信号量：意味着一个交换链图像已被获取，其可安全渲染
    VkSemaphore                 swapchainImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    // 信号量：意味着图像渲染完毕，其可安全呈现（数量为交换链图像数量）
    VkSemaphore*                renderFinishedSemaphores;
    // 栅栏：表示一个渲染进行中的帧（执行命令缓冲区），用于防止命令缓冲区发生竞态
    VkFence                     frameInFlightFences[MAX_FRAMES_IN_FLIGHT];

} RendererContext;