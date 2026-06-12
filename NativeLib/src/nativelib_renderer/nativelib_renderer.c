/// nativelib_vkrenderer 对外暴露的接口，该接口只尝试暴露最低限度抽象的 Vulkan API 的细节
#include "nativelib_renderer.h"

#include <pthread.h>

#include "../common/log.h"
#include "context/renderer_context.h"
#include "context/structs/renderer_context_structs.h"
#include "data_structs/renderer_data_structs.h"

static bool g_initialized = false;

static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static RendererContext* g_pContext = NULL;

static bool g_isFramebufferResized = false;

static void log_lock_function(bool isLock, void* pMutex);


EX_API bool vrdrInitialize(
    uint32_t                windowExtensionCount,
    const char**            windowExtensionStrings,
    SurfaceCreateHelperFunc create_window_surface_helper,
    int                     windowFramebufferWidth,
    int                     windowFramebufferHeight
)
{
    // 配置 log 库
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif
    log_set_lock(log_lock_function, &g_log_mutex);

    if(g_initialized)
        return false;

    // 为渲染器上下文分配内存
    g_pContext = rctxNewRendererContext();

    if (!g_pContext)
        return false;

    // 构建渲染器上下文
    if (!rctxCreateRendererContext(g_pContext,
             windowExtensionCount,
             windowExtensionStrings,
             create_window_surface_helper,
             windowFramebufferWidth,
             windowFramebufferHeight))
    {
        rctxDestroyRendererContext(g_pContext);
        return false;
    }

    /***
        RenderingService.Initialize 中调用
        WindowingService.SetFramebufferSizeCallback(this.vkrdrFrambufferSizeCallback) 
        以设置回调函数 
    ***/

    g_initialized = true;

    return true;
}

static void log_lock_function(bool isLock, void* pMutex)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)pMutex;

    if (isLock)
    {
        pthread_mutex_lock(mutex);
    }
    else
    {
        pthread_mutex_unlock(mutex);
    }
}


EX_API void vkrdrFramebufferResizeCallback(Window* window, int width, int height)
{
    // 避免未使用参数警告
    (void)window;

    log_debug("%s(): width = %d, hight = %d", __func__, width, height);

    // 设置该全局变量以表示窗口的帧缓冲区大小被更改
    g_isFramebufferResized = true;
}


EX_API bool vrdrCreateStaticVertexBuffer(
    size_t              dataSize,
    const void*         pVertexData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
)
{
    return rctxCreateStaticBuffer(g_pContext,
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               dataSize,
               0,
               dataSize,
               pVertexData,
               outBuffer,
               outAllocation);
}


EX_API bool vrdrCreateStaticIndexBuffer(
    uint32_t            dataSize,
    const uint32_t*     pIndexData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
)
{
    return rctxCreateStaticBuffer(g_pContext,
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               dataSize,
               0,
               dataSize,
               pIndexData,
               outBuffer,
               outAllocation);
}


EX_API uint32_t vrdrGetMinimalUniformBufferOffsetAlignment()
{
    return g_pContext->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
}


EX_API bool vrdrCreateDynamicUniformBuffer(
    size_t              bufferSize,
    uint32_t            dataOffset,
    size_t              dataSize,
    const void*         pUniformData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
)
{
    return rctxCreateDynamicBuffer(g_pContext,
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               bufferSize,
               dataOffset,
               dataSize,
               pUniformData,
               outBuffer,
               outAllocation);
}


EX_API void vrdrUpdateUniformBuffer(
    size_t          bufferSize,
    uint32_t        dataOffset,
    size_t          dataSize,
    void*           pData,
    VmaAllocation   allocation
)
{
    rctxUpdateDynamicBuffer(g_pContext,
        bufferSize,
        dataOffset,
        dataSize,
        pData,
        allocation);
}


EX_API bool vrdrRequestDestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
    return rctxRequestDestroyBuffer(g_pContext, buffer, allocation);
}


EX_API void vrdrWaitIdle()
{
    rctxWaitIdle(g_pContext);
}


EX_API void vrdrDeletionListDangerousFlushALL()
{
    rctxDeletionListDangerousFlushALL(g_pContext);
}


EX_API void vrdrDangerousDestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
    rctxDangerousDestroyBuffer(g_pContext, buffer, allocation);
}


EX_API bool vrdrAllocateCameraDescriptorSet(
    size_t              cameraUniformBufferOffset,
    size_t              cameraUniformBufferRange,
    const VkBuffer      cameraUniformBuffer,
    VkDescriptorSet*    outDescriptorSet
)
{
    VkDescriptorBufferInfo cameraUniformBufferInfo = {};
    cameraUniformBufferInfo.offset = cameraUniformBufferOffset;
    cameraUniformBufferInfo.range  = cameraUniformBufferRange;
    cameraUniformBufferInfo.buffer = cameraUniformBuffer;

    return rctxAllocateCameraDescriptorSet(g_pContext,
               &cameraUniformBufferInfo,
               outDescriptorSet);
}


EX_API bool vrdrAllocateDrawItemDescriptorSet(
    size_t              drawItemUniformBufferOffset,
    size_t              drawItemUniformBufferRange,
    const VkBuffer      drawItemUniformBuffer,
    VkDescriptorSet*    outDescriptorSet
)
{
    VkDescriptorBufferInfo drawItemUniformBufferInfo = {};
    drawItemUniformBufferInfo.offset = drawItemUniformBufferOffset;
    drawItemUniformBufferInfo.range  = drawItemUniformBufferRange;
    drawItemUniformBufferInfo.buffer = drawItemUniformBuffer;

    return rctxAllocateDrawItemDescriptorSet(g_pContext,
               &drawItemUniformBufferInfo,
               outDescriptorSet);
}


EX_API void vrdrBeginFrame()
{
    rctxBeginFrame(g_pContext);
}


EX_API void vrdrDrawFrame(
    MainRenderPassDrawInfo  *pMainRenderPassDrawInfo,
    int                     windowFramebufferWidth,
    int                     windowFramebufferHeight
)
{
    rctxDrawFrame(g_pContext,
        g_isFramebufferResized,
        windowFramebufferWidth,
        windowFramebufferHeight,
        pMainRenderPassDrawInfo);

    g_isFramebufferResized = false;
}


EX_API void vrdrEndFrame()
{
    rctxEndFrame(g_pContext);
}


EX_API void vrdrTerminate()
{
    if (!g_initialized)
        return;

    rctxWaitIdle(g_pContext);

    // 对所有的销毁队列进行销毁刷新
    rctxDeletionListDangerousFlushALL(g_pContext);

    rctxDestroyRendererContext(g_pContext);

    g_initialized = false;
}

