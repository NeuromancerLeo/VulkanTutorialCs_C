#include "render_context.h"


RenderContext* new_render_context()
{
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif

    // 分配堆内存
    RenderContext* pContext = (RenderContext*)calloc(1, sizeof(RenderContext));
    if (!pContext) return NULL;

    return pContext;
}

bool create_render_context(GLFWwindow* window, RenderContext* pContext)
{
    log_info("开始构建渲染上下文...");

    pContext->window = window;                          // 保存窗口句柄

    pContext->instance = createInstance();              // 创建 Vk 实例
    if (pContext->instance == VK_NULL_HANDLE)
        return false;

    pContext->surface = createSurface(pContext->instance, pContext->window); 
    if (pContext->surface == VK_NULL_HANDLE)            // 创建窗口表面 
        return false;
    
    pContext->physicalDevice = pickPhysicalDevice(pContext->instance, pContext->surface);
    if (pContext->physicalDevice == VK_NULL_HANDLE)     // 选取物理设备
        return false;
    
    pContext->device = createLogicalDevice(pContext->physicalDevice,    // 创建 Vk 设备
                           pContext->surface,
                           &pContext->graphicsQueue,
                           &pContext->presentationQueue);
    if (pContext->device == VK_NULL_HANDLE)
        return false;

    pContext->swapchain = createSwapchain(pContext->window,    // 为窗口（表面）创建交换链
                              pContext->surface,
                              pContext->physicalDevice,
                              pContext->device,
                              &pContext->swapchainImageCount,
                              &pContext->swapchainImages,
                              &pContext->swapchainImageFormat,
                              &pContext->swapchainExtent);
    if (pContext->swapchain == VK_NULL_HANDLE)
        return false;

    pContext->swapchainImageViews = createSwapchainImageViews(pContext->device,
                                        pContext->swapchainImageFormat,       
                                        pContext->swapchainImageCount,    // 创建交换链的
                                        pContext->swapchainImages);       // 图形视图
    if (!pContext->swapchainImageViews)
        return false;

    log_info("渲染上下文构建完毕.");

    return true;
}

void destroy_render_context(RenderContext* pContext)
{
    log_info("销毁渲染上下文...");

    if (!pContext)
    {
        log_info("%s : 给定渲染上下文地址无效, 退出.", __func__);
        return;
    }

    if (pContext->instance == VK_NULL_HANDLE)
    {
        log_info("%s : 给定渲染上下文不含有效的 VkInstance 句柄，不会销毁任何内容，退出.",
            __func__);
        return;
    }

    // TODO: vkDeviceWaitIdle

    if (pContext->swapchainImageViews)                             // 销毁交换链图像视图
        destroySwapchainImageViews(pContext->device,               // 并释放其数组占用的
            pContext->swapchainImageCount,                         // 内存
            &pContext->swapchainImageViews);

    if (pContext->swapchain != VK_NULL_HANDLE)                     // 销毁交换链
        destroySwapchain(pContext->device, pContext->swapchain);
    
    if (pContext->swapchainImages)                                 // 释放交换链图像数组
    {                                                              // 占用的内存
        free(pContext->swapchainImages);
        pContext->swapchainImages = NULL;
    }

    if (pContext->device != VK_NULL_HANDLE)                        // 销毁 Vk 设备
        destroyLogicalDevice(pContext->device);
    
    if (pContext->surface != VK_NULL_HANDLE)                       // 销毁窗口表面
        destroySurface(pContext->instance, pContext->surface);

    destroyInstance(pContext->instance);                           // 销毁 Vk 实例

    free(pContext);                                                // 释放渲染上下文结构体
    pContext = NULL;                                               // 占用的内存

    log_info("销毁渲染上下文完毕.");

    return;
}