#include "render_context.h"


RenderContext* new_render_context()
{
    RenderContext* pContext = (RenderContext*)malloc(sizeof(RenderContext));
    if (!pContext) return NULL;

    // 清 0 初始化
    memset(pContext, 0, sizeof(RenderContext));

    return pContext;
}

bool create_render_context(GLFWwindow* window, RenderContext* pContext)
{
    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        "开始构建渲染上下文...\n",
        __DATE__, __TIME__);

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

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        "渲染上下文构建完毕.\n",
        __DATE__, __TIME__);

    return true;
}

void destroy_render_context(RenderContext* pContext)
{
    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        "销毁渲染上下文...\n",
        __DATE__, __TIME__);

    if (!pContext)
    {
        fprintf(stdout, "%s : 给定渲染上下文地址无效, 退出.\n", __func__);
        return;
    }

    if (pContext->instance == VK_NULL_HANDLE)
    {
        fprintf(stdout, 
            "%s : 给定渲染上下文不含有效的 VkInstance 句柄，不会销毁任何内容并退出.\n",
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

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        "销毁渲染上下文完毕.\n",
        __DATE__, __TIME__);

    return;
}