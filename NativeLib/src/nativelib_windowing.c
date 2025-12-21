#include "nativelib_windowing.h"
#include "ansi_esc.h"

EX_API GLFWwindow* initializeWindow(int width, int height, const char* title)
{
    if (!glfwInit())
    {
        fprintf(stderr, ESC_FCOLOR_BRIGHT_RED "Failed to initialize glfw!" ESC_RESET);
        return NULL;
    }
        
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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