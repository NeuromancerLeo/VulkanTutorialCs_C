#include "nativelib.h"

#include <GLFW/glfw3.h>
#include <stdio.h>

/// @brief 初始化 GLFW 库并创建一个窗口.
/// @param width 窗口的宽（屏幕坐标系下）
/// @param height 窗口的高（屏幕坐标系下）
/// @param title 窗口标题
/// @return 一个创建好的窗口的句柄，或者 `NULL` 当发生错误时
EX_API GLFWwindow* initializeWindow(int width, int height, const char* title)
{
    if (!glfwInit())
    {
        fprintf(stderr, "failed to initialize glfw!");
        return NULL;
    }
        
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(width, height, title, NULL, NULL);
}

/// @brief 销毁窗口.
/// @param window 要销毁的窗口
/// @return 
EX_API void destroyWindow(GLFWwindow* window)
{
    glfwDestroyWindow(window);
}

/// @brief 检查给定窗口的关闭标志.
/// @param window 给定窗口
/// @return 关闭标志的值.
EX_API int windowShouldClose(GLFWwindow* window)
{
    return glfwWindowShouldClose(window);
}

/// @brief 处理所有挂起的窗口事件.
EX_API void pollEvents(void)
{
    glfwPollEvents();
}

/// @brief 终止 GLFW 库.
/// @param  
/// @return 
EX_API void terminate(void)
{
    glfwTerminate();
}