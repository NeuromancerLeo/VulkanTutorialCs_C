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

    pContext->window = window;

    pContext->instance = createInstance();
    if (pContext->instance == VK_NULL_HANDLE)
        return false;

    pContext->surface = createSurface(pContext->instance, pContext->window);
    if (pContext->surface == VK_NULL_HANDLE)
        return false;
    
    pContext->physicalDevice = pickPhysicalDevice(pContext->instance, pContext->surface);
    if (pContext->physicalDevice == VK_NULL_HANDLE)
        return false;
    
    pContext->device = createLogicalDevice(pContext->physicalDevice, 
                          pContext->surface,
                          &pContext->graphicsQueue,
                          &pContext->presentationQueue);
    if (pContext->device == VK_NULL_HANDLE)
        return false;

    pContext->swapchain = createSwapchain(pContext->window,
                             pContext->surface,
                             pContext->physicalDevice,
                             pContext->device,
                             &pContext->swapchainImageCount,
                             &pContext->swapchainImages,
                             &pContext->swapchainImageFormat,
                             &pContext->swapchainExtent);
    if (pContext->swapchain == VK_NULL_HANDLE)
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

    if (!pContext || pContext->instance == VK_NULL_HANDLE)
    {
        fprintf(stdout, "给定渲染上下文无效, 停止销毁.\n");
        return;
    }

    // TODO: vkDeviceWaitIdle

    if (pContext->swapchain != VK_NULL_HANDLE)
        destroySwapchain(pContext->device, pContext->swapchain);
    
    if (pContext->swapchainImages)
    {
        free(pContext->swapchainImages);
        pContext->swapchainImages = NULL;
    }

    if (pContext->device != VK_NULL_HANDLE)
        destroyLogicalDevice(pContext->device);
    
    if (pContext->surface != VK_NULL_HANDLE)
        destroySurface(pContext->instance, pContext->surface);

    destroyInstance(pContext->instance);

    free(pContext);
    pContext = NULL;

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        "销毁渲染上下文完毕.\n",
        __DATE__, __TIME__);

    return;
}