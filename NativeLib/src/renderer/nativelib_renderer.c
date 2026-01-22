/// 对外暴露的接口，该接口应该永远不暴露任何具体图形 API 的细节

#include "nativelib_renderer.h"

static RendererContext* g_context = NULL;


EX_API bool rendererInitialize(GLFWwindow* window)
{
    // 为渲染器上下文分配内存
    g_context = new_renderer_context();
    // 构建渲染器上下文
    if (!create_renderer_context(g_context, window))
    {
        destroy_renderer_context(g_context);
        return false;
    }

    return true;
}


EX_API bool rendererReady()
{
    // 为绘制三角形录制命令缓冲区
    if (!triangle_record_command_buffer(g_context, 0))
        return false;

    return true;
}


EX_API void rendererBeginFrame()
{

}


EX_API void rendererEndFrame()
{

}


EX_API void rendererRelease()
{
    destroy_renderer_context(g_context);
}

