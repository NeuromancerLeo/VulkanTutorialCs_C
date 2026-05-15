/// 对外暴露的接口，该接口应该永远不暴露任何具体图形 API 的细节

#include "nativelib_renderer.h"

static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static RendererContext* g_pContext = NULL;

static bool g_isFramebufferResized = false;

static void log_lock_function(bool isLock, pthread_mutex_t* pMutex);
static void frame_buffer_resize_call_back(GLFWwindow* window, int width, int height);


EX_API bool rendererInitialize(GLFWwindow* window)
{
    // 配置 log 库
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif
    log_set_lock(log_lock_function, &g_log_mutex);

    // 为渲染器上下文分配内存
    g_pContext = rctxNewRendererContext();
    // 构建渲染器上下文
    if (!rctxCreateRendererContext(g_pContext, window))
    {
        rctxDestroyRendererContext(g_pContext);
        return false;
    }

    // 这里设置一些 GLFW 回调函数
    glfwSetFramebufferSizeCallback(window, frame_buffer_resize_call_back);

    return true;
}

static void log_lock_function(bool isLock, pthread_mutex_t* pMutex)
{
    if (isLock)
    {
        pthread_mutex_lock(pMutex);
    }
    else
    {
        pthread_mutex_unlock(pMutex);
    }
}

static void frame_buffer_resize_call_back(GLFWwindow* window, int width, int height)
{
    // 避免未使用参数警告
    (void)window;

    log_debug("%s(): width = %d, hight = %d", __func__, width, height);

    // 设置该全局变量以表示 GLFW 的窗口帧缓冲区大小被更改
    g_isFramebufferResized = true;
}

// TODO: C# 自行接收并包装成 BufferResource 类
static VkBuffer g_triangleBuffer = VK_NULL_HANDLE; // 临时
static VmaAllocation g_triangleBufferAllocation = VK_NULL_HANDLE; // 临时
static VkBuffer g_triangleIndexBuffer = VK_NULL_HANDLE; // 临时
static VmaAllocation g_triangleIndexBufferAllocation = VK_NULL_HANDLE; // 临时

static VkBuffer g_globalUniformBuffer = VK_NULL_HANDLE;
// static UnlitMaterial unlitMaterial0 = VK_NULL_HANDLE;
static VkBuffer g_unlitDrawItemUniformsBuffer = VK_NULL_HANDLE;
EX_API bool rendererReady()
{
    // 模拟 C# 端通过该 DLL 函数传入顶点数据
    const VertexData triangleVerticesData[] = {  // interleaving vertex attributes
        {.position = {0.0f, -0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}},  // 最上方的点
        {.position = {0.5f,  0.5f, 0.0f}, .color = {0.0f, 0.0f, 0.0f}},  // 右下角的点
        {.position = {-0.5f, 0.5f, 0.0f}, .color = {0.0f, 0.0f, 0.0f}},  // 左下角的点  // 顺时针为正面
        {.position = {0.0f,  0.6f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}},  // 偏下方的点
        {.position = {0.2f,  0.8f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},  // 最下方偏右的点
        {.position = {-0.2f,  0.8f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}}, // 最下方偏左的点
        {.position = {-0.9f, -0.1f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}}, // 矩形，左上 6
        {.position = {-0.7f, -0.1f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}}, // 矩形，右上 7
        {.position = {-0.9f, 0.1f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},  // 矩形，左下 8
        {.position = {-0.7f, 0.1f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},  // 矩形，右下 9
        {.position = {0.84f, -0.2f, 0.0f}, .color = {0.1f, 0.1f, 0.4f}}, // 不规则矩形，左上 10
        {.position = {0.99f, -0.3f, 0.0f}, .color = {0.1f, 0.1f, 0.4f}}, // 不规则矩形，右上 11
        {.position = {0.8f, 0.4f, 0.0f}, .color = {0.1f, 0.1f, 0.4f}},  // 不规则矩形，左下 12
        {.position = {0.9f, 0.1f, 0.0f}, .color = {0.1f, 0.1f, 0.4f}}   // 不规则矩形，右下 13
    };

    uint32_t triangleVertexIndices[] = {
        6, 9, 8,
        6, 7, 9,
        10, 13, 12,
        10, 11, 13
    };

    // TODO: 返回 VkBuffer 资源句柄给 C# 端持有，让其管理负责资源的生命周期！
    if (!rendererCreateStaticVertexBuffer(sizeof(triangleVerticesData),
             triangleVerticesData,
             &g_triangleBuffer,
             &g_triangleBufferAllocation)
        || !rendererCreateStaticIndexBuffer(sizeof(triangleVertexIndices),
               triangleVertexIndices,
               &g_triangleIndexBuffer,
               &g_triangleIndexBufferAllocation))
    {
        return false;
    }

    return true;
}


EX_API bool rendererCreateStaticVertexBuffer(
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


EX_API bool rendererCreateStaticIndexBuffer(
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


EX_API uint32_t rendererGetMinimalUniformBufferOffsetAlignment()
{
    return g_pContext->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
}


EX_API bool rendererCreateDynamicUniformBuffer(
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


EX_API bool rendererUpdateUniformBuffer()
{
    return true;
}


EX_API void rendererRequestDestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
    rctxRequestDestroyBuffer(g_pContext, buffer, allocation);
}


EX_API void rendererDestroyBuffer()
{

}


EX_API void rendererBeginFrame()
{
    rctxBeginFrame(g_pContext);
}


EX_API void rendererDrawFrame()
{
    // 临时：准备绘制信息
    /**** Triangle 管线部分 ****/

    // 模拟 C# 端通过该 DLL 函数传入的顶点数据信息
    DrawItemInfo triangleDrawItemInfo01 = {};
    triangleDrawItemInfo01.vertexCount   = 3;
    triangleDrawItemInfo01.instanceCount = 1;
    triangleDrawItemInfo01.firstVertex   = 0;
    triangleDrawItemInfo01.firstInstance = 0;

    DrawItemInfo triangleDrawItemInfo02 = {};
    triangleDrawItemInfo02.vertexCount   = 3;
    triangleDrawItemInfo02.instanceCount = 1;
    triangleDrawItemInfo02.firstVertex   = 3;
    triangleDrawItemInfo02.firstInstance = 0;

    DrawItemInfo triangleDrawItemInfos[] = {
        triangleDrawItemInfo01, triangleDrawItemInfo02
    };

    IndexedDrawItemInfo triangleIndexedDrawItemInfo01 = {};
    triangleIndexedDrawItemInfo01.indexCount    = 6;
    triangleIndexedDrawItemInfo01.instanceCount = 1;
    triangleIndexedDrawItemInfo01.firstIndex    = 0;
    triangleIndexedDrawItemInfo01.vertexOffset  = 0;
    triangleIndexedDrawItemInfo01.firstInstance = 0;

    IndexedDrawItemInfo triangleIndexedDrawItemInfo02 = {};
    triangleIndexedDrawItemInfo02.indexCount    = 6;
    triangleIndexedDrawItemInfo02.instanceCount = 1;
    triangleIndexedDrawItemInfo02.firstIndex    = 6;
    triangleIndexedDrawItemInfo02.vertexOffset  = 0;
    triangleIndexedDrawItemInfo02.firstInstance = 0;

    IndexedDrawItemInfo triangleIndexedDrawItemInfos[] = {
        triangleIndexedDrawItemInfo01, triangleIndexedDrawItemInfo02
    };

    // 准备 triangle 管线的顶点集信息
    TrianglePipelineBindingDrawInfo triangleBindingDrawInfo0 = {};
    triangleBindingDrawInfo0.vertexBuffer          = g_triangleBuffer;  // 对应了绑定
    triangleBindingDrawInfo0.vertexOffset          = 0;
    triangleBindingDrawInfo0.indexBuffer           = g_triangleIndexBuffer;
    triangleBindingDrawInfo0.indexOffset           = 0;
    triangleBindingDrawInfo0.drawItemCount         = 2;  // 对应了该绑定对应绘制
    triangleBindingDrawInfo0.pDrawItemInfos        = triangleDrawItemInfos;
    triangleBindingDrawInfo0.indexedDrawItemCount  = 2;
    triangleBindingDrawInfo0.pIndexedDrawItemInfos = triangleIndexedDrawItemInfos;

    TrianglePipelineDrawInfo trianglePipelineDrawInfo = {};
    trianglePipelineDrawInfo.bindingCount = 1;   // 对应了绑定次数
    trianglePipelineDrawInfo.pBindingDrawInfos = &triangleBindingDrawInfo0;

    // 为 给 Triangle 管线准备的绘制信息 新建一个 PipelineDrawTask
    PipelineDrawTask trianglePipelineDrawTask = {};
    trianglePipelineDrawTask.pipelineType      = RCTX_PIPELINE_TYPE_TRIANGLE;
    trianglePipelineDrawTask.pPipelineDrawInfo = &trianglePipelineDrawInfo;

    MainRenderPassDrawInfo mainRenderPassDrawInfo = {};
    // 设置 Default 子通道
    // 只绑定一个管线任务，也就是 Triangle 管线
    mainRenderPassDrawInfo.defaultPass.pipelineDrawTaskCount = 1;
    mainRenderPassDrawInfo.defaultPass.pipelineDrawTasks = &trianglePipelineDrawTask;

    rctxDrawFrame(g_pContext, g_isFramebufferResized, &mainRenderPassDrawInfo);

    g_isFramebufferResized = false;
}


EX_API void rendererEndFrame()
{
    rctxEndFrame(g_pContext);
}


EX_API void rendererRelease()
{
    // TODO: 不是好主意
    vkDeviceWaitIdle(g_pContext->device);    

    // 临时
    rctxDestroyBuffer(g_pContext, g_triangleBuffer, g_triangleBufferAllocation);
    rctxDestroyBuffer(g_pContext, g_triangleIndexBuffer, g_triangleIndexBufferAllocation);

    rctxDestroyRendererContext(g_pContext);   
}

