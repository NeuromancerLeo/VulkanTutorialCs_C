#include "nativelib_windowing.h"


EX_API GLFWwindow* initializeWindow(int width, int height, const char* title)
{
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif

    if (!glfwInit())
    {
        log_error(ESC_FCOLOR_BRIGHT_RED "Failed to initialize glfw!" ESC_RESET);
        return NULL;
    }
        
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    return glfwCreateWindow(width, height, title, NULL, NULL);
}


EX_API void destroyWindow(GLFWwindow* window)
{
    glfwDestroyWindow(window);
}


EX_API int windowShouldClose(GLFWwindow* window)
{
    return glfwWindowShouldClose(window);
}


EX_API void pollEvents(void)
{
    glfwPollEvents();
}


EX_API void terminate(void)
{
    glfwTerminate();
}