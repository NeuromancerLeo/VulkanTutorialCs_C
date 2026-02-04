/// 对外暴露的接口，该接口应该永远不暴露任何具体图形 API 的细节

#include "nativelib_renderer.h"

static RendererContext* g_context = NULL;

static bool g_isFramebufferResized = false;

static void frame_buffer_resize_call_back(GLFWwindow* window, int width, int height);


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


EX_API bool rendererReady()
{
    return true;
}


EX_API void rendererDrawFrame()
{
    triangle_draw_frame(g_context, g_isFramebufferResized);

    g_isFramebufferResized = false;
}


EX_API void rendererRelease()
{
    destroy_renderer_context(g_context);
}

