/// 对外暴露的接口，该接口应该永远不暴露任何具体图形 API 的细节

#include "nativelib_renderer.h"

static RendererContext* g_context = NULL;

static bool g_isFramebufferResized = false;

static void frame_buffer_resize_call_back(GLFWwindow* window, int width, int height);


EX_API bool rendererInitialize(GLFWwindow* window)
{
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

static BufferResource g_triangleBuffer = {};
EX_API bool rendererReady()
{
    // 模拟 C# 端通过该 DLL 函数传入顶点数据
    const VertexData verticesData[] = {       // interleaving vertex attributes
        {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},    // 最上方的点
        {{0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}},    // 右下角的点
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},    // 左下角的点  // 顺时针为正面
        {{0.0f,  0.6f}, {1.0f, 1.0f, 1.0f}},     // 偏下方的点
        {{0.2f,  0.8f}, {0.0f, 0.0f, 1.0f}},    // 最下方偏右的点
        {{-0.2f,  0.8f}, {0.0f, 1.0f, 0.0f}}     // 最下方偏左的点
    };

    // if (!triangle_allocate_and_fill_vertex_buffer(g_context,
    //          sizeof(verticesData),
    //          verticesData))
    //     return false;

    // TODO: 返回 VkBuffer 资源句柄给 C# 端持有，让其管理负责资源的生命周期！
    g_triangleBuffer = rctxCreateAndFillStaticBuffer(g_context,
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           sizeof(verticesData),
                           verticesData);

    return true;
}


EX_API BufferResource rendererCreateStaticVertexBuffer(
    size_t              dataSize,
    const VertexData*   pVertiesData
)
{
    return rctxCreateAndFillStaticBuffer(g_context,
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               dataSize,
               pVertiesData);
}


EX_API BufferResource rendererCreateStaticIndexBuffer(
    size_t              dataSize,
    const IndexData*    pIndicesData
)
{
    return rctxCreateAndFillStaticBuffer(g_context,
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               dataSize,
               pIndicesData);
}


EX_API BufferResource rendererCreateDynamicUniformBuffer()
{
    return (BufferResource){0};
}


EX_API BufferResource rendererUpdateUniformBuffer()
{
    return (BufferResource){0};
}


EX_API void rendererDestroyBuffer()
{

}


EX_API void rendererDrawFrame()
{
    // 准备绘制信息
    MainRenderPassPipelinesDrawInfo mainRenderPassPipelinesDrawInfo = {};
    // TODO: 在渲染器上下文中建立一个句柄表存储对应的 vertex buffer，然后 rctxDrawFrame 内部会根据从外界（C#端）传入的句柄或标识符来查找 VkBuffer 对象并使用.
    
    /**** 三角形管线部分 ****/
    mainRenderPassPipelinesDrawInfo.triangle.vertexBufferInfo.buffer = 
        g_triangleBuffer.buffer;
    mainRenderPassPipelinesDrawInfo.triangle.vertexBufferInfo.offset = 0;

    // 模拟 C# 端通过该 DLL 函数传入的顶点数据信息
    DrawItemInfo drawItemInfo1 = {};
    drawItemInfo1.vertexCount   = 3;
    drawItemInfo1.instanceCount = 1;
    drawItemInfo1.firstVertex   = 0;
    drawItemInfo1.firstInstance = 0;

    DrawItemInfo drawItemInfo2 = {};
    drawItemInfo2.vertexCount   = 3;
    drawItemInfo2.instanceCount = 1;
    drawItemInfo2.firstVertex   = 3;
    drawItemInfo2.firstInstance = 0;

    DrawItemInfo drawItemInfos[] = {
        drawItemInfo1, drawItemInfo2
    };

    mainRenderPassPipelinesDrawInfo.triangle.vertexBufferInfo.drawItemCount  = 2;
    mainRenderPassPipelinesDrawInfo.triangle.vertexBufferInfo.pDrawItemInfos = 
        drawItemInfos;

    rctxDrawFrame(g_context, g_isFramebufferResized, &mainRenderPassPipelinesDrawInfo);

    g_isFramebufferResized = false;
}


EX_API void rendererRelease()
{
    // TODO: 不是好主意
    vkDeviceWaitIdle(g_context->device);    

    // 临时
    rctxDestroyBuffer(g_context, g_triangleBuffer);

    rctxDestroyRendererContext(g_context);   
}

