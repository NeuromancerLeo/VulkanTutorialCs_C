/// 对外暴露的接口，该接口应该永远不暴露任何具体图形 API 的细节

#include "nativelib_renderer.h"

static RenderContext* g_context = NULL;


EX_API bool rendererInitialize(GLFWwindow* window)
{
    // 为渲染上下文分配内存
    g_context = new_render_context();
    // 构建渲染上下文
    if (!create_render_context(window, g_context))
    {
        destroy_render_context(g_context);
        return false;
    }

    return true;
}


EX_API void rendererReady()
{
    // 配置渲染设置，仅在初始化成功后被调用一次
}


EX_API void rendererBeginFrame()
{

}


EX_API void rendererEndFrame()
{

}


EX_API void rendererRelease()
{
    destroy_render_context(g_context);
}

