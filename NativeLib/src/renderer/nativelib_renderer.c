/// 对外暴露的接口，该接口应该永远不暴露任何具体图形 API 的细节

#include "nativelib_renderer.h"

static RendererContext* g_context = NULL;

static bool g_isFramebufferResized = false;

static void frame_buffer_resize_call_back(GLFWwindow* window, int width, int height);


EX_API bool rendererInitialize(GLFWwindow* window)
{
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif

    // 为渲染器上下文分配内存
    g_context = rctxNewRendererContext();
    // 构建渲染器上下文
    if (!rctxCreateRendererContext(g_context, window))
    {
        rctxDestroyRendererContext(g_context);
        return false;
    }

    // 这里设置一些 GLFW 回调函数
    glfwSetFramebufferSizeCallback(window, frame_buffer_resize_call_back);

    return true;
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
EX_API bool rendererReady()
{
    // 模拟 C# 端通过该 DLL 函数传入顶点数据
    const VertexData verticesData[] = {  // interleaving vertex attributes
        {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},      // 最上方的点
        {{0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}},      // 右下角的点
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},      // 左下角的点  // 顺时针为正面
        {{0.0f,  0.6f}, {1.0f, 1.0f, 1.0f}},      // 偏下方的点
        {{0.2f,  0.8f}, {0.0f, 0.0f, 1.0f}},      // 最下方偏右的点
        {{-0.2f,  0.8f}, {0.0f, 1.0f, 0.0f}},     // 最下方偏左的点
        {{-0.9f, -0.1f}, {0.0f, 1.0f, 0.0f}}, // 矩形，左上 6
        {{-0.7f, -0.1f}, {0.0f, 1.0f, 0.0f}}, // 矩形，右上 7
        {{-0.9f, 0.1f}, {0.0f, 1.0f, 0.0f}},  // 矩形，左下 8
        {{-0.7f, 0.1f}, {0.0f, 1.0f, 0.0f}},  // 矩形，右下 9
        {{0.84f, -0.2f}, {0.1f, 0.1f, 0.4f}}, // 不规则矩形，左上 10
        {{0.99f, -0.3f}, {0.1f, 0.1f, 0.4f}}, // 不规则矩形，右上 11
        {{0.8f, 0.4f}, {0.1f, 0.1f, 0.4f}},  // 不规则矩形，左下 12
        {{0.9f, 0.1f}, {0.1f, 0.1f, 0.4f}}   // 不规则矩形，右下 13
    };

    uint32_t vertexIndices[] = {
        6, 9, 8,
        6, 7, 9,
        10, 13, 12,
        10, 11, 13
    };

    // TODO: 返回 VkBuffer 资源句柄给 C# 端持有，让其管理负责资源的生命周期！
    if (!rendererCreateStaticVertexBuffer(sizeof(verticesData),
             verticesData,
             &g_triangleBuffer,
             &g_triangleBufferAllocation)
        || !rendererCreateStaticIndexBuffer(sizeof(vertexIndices),
               vertexIndices,
               &g_triangleIndexBuffer,
               &g_triangleIndexBufferAllocation))
    {
        return false;
    }

    return true;
}


EX_API bool rendererCreateStaticVertexBuffer(
    uint32_t            dataSize,
    const VertexData*   pVertiesData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
)
{
    return rctxCreateAndFillStaticBuffer(g_context,
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               dataSize,
               pVertiesData,
               outBuffer,
               outAllocation);
}


EX_API bool rendererCreateStaticIndexBuffer(
    uint32_t            dataSize,
    const uint32_t*     pIndicesData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
)
{
    return rctxCreateAndFillStaticBuffer(g_context,
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               dataSize,
               pIndicesData,
               outBuffer,
               outAllocation);
}


EX_API bool rendererCreateDynamicUniformBuffer()
{
    return true;
}


EX_API bool rendererUpdateUniformBuffer()
{
    return true;
}


EX_API void rendererDestroyBuffer()
{

}


EX_API void rendererDrawFrame()
{
    // 准备绘制信息
    MainRenderPassPipelinesDrawInfo mainRenderPassPipelinesDrawInfo = {};
    
    /**** 三角形管线部分 ****/

    // 模拟 C# 端通过该 DLL 函数传入的顶点数据信息
    DrawItemInfo drawItemInfo01 = {};
    drawItemInfo01.vertexCount   = 3;
    drawItemInfo01.instanceCount = 1;
    drawItemInfo01.firstVertex   = 0;
    drawItemInfo01.firstInstance = 0;

    DrawItemInfo drawItemInfo02 = {};
    drawItemInfo02.vertexCount   = 3;
    drawItemInfo02.instanceCount = 1;
    drawItemInfo02.firstVertex   = 3;
    drawItemInfo02.firstInstance = 0;

    DrawItemInfo drawItemInfos[] = {
        drawItemInfo01, drawItemInfo02
    };

    IndexedDrawItemInfo indexedDrawItemInfo01 = {};
    indexedDrawItemInfo01.indexCount    = 6;
    indexedDrawItemInfo01.instanceCount = 1;
    indexedDrawItemInfo01.firstIndex    = 0;
    indexedDrawItemInfo01.vertexOffset  = 0;
    indexedDrawItemInfo01.firstInstance = 0;

    IndexedDrawItemInfo indexedDrawItemInfo02 = {};
    indexedDrawItemInfo02.indexCount    = 6;
    indexedDrawItemInfo02.instanceCount = 1;
    indexedDrawItemInfo02.firstIndex    = 6;
    indexedDrawItemInfo02.vertexOffset  = 0;
    indexedDrawItemInfo02.firstInstance = 0;

    IndexedDrawItemInfo indexedDrawItemInfos[] = {
        indexedDrawItemInfo01, indexedDrawItemInfo02
    };

    // 准备 triangle 管线的顶点集信息
    TrianglePipelineBindingBuffersInfo buffersBindingInfo = {};
    buffersBindingInfo.vertexBuffer   = g_triangleBuffer; // 对应了绑定
    buffersBindingInfo.vertexOffset   = 0;
    buffersBindingInfo.indexBuffer    = g_triangleIndexBuffer;
    buffersBindingInfo.indexOffset    = 0;
    buffersBindingInfo.drawItemCount  = 2;                // 对应了该绑定对应绘制
    buffersBindingInfo.pDrawItemInfos = drawItemInfos;
    buffersBindingInfo.indexedDrawItemCount  = 2;
    buffersBindingInfo.pIndexedDrawItemInfos = indexedDrawItemInfos;

    mainRenderPassPipelinesDrawInfo.triangle.buffersBindingCount  = 1;  // 对应了绑定次数
    mainRenderPassPipelinesDrawInfo.triangle.pBindingBuffersInfos = &buffersBindingInfo;

    rctxDrawFrame(g_context, g_isFramebufferResized, &mainRenderPassPipelinesDrawInfo);

    g_isFramebufferResized = false;
}


EX_API void rendererRelease()
{
    // TODO: 不是好主意
    vkDeviceWaitIdle(g_context->device);    

    // 临时
    rctxDestroyBuffer(g_context, g_triangleBuffer, g_triangleBufferAllocation);
    rctxDestroyBuffer(g_context, g_triangleIndexBuffer, g_triangleIndexBufferAllocation);

    rctxDestroyRendererContext(g_context);   
}

