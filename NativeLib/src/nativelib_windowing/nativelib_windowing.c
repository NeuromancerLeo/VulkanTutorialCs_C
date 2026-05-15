#include "nativelib_windowing.h"

static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void log_lock_function(bool isLock, pthread_mutex_t* pMutex);


EX_API GLFWwindow* initializeWindow(int width, int height, const char* title)
{
    // 配置 log 库
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif
    log_set_lock(log_lock_function, &g_log_mutex);

    if (!glfwInit())
    {
        log_error(ESC_FCOLOR_BRIGHT_RED "Failed to initialize glfw!" ESC_RESET);
        return NULL;
    }
        
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    return glfwCreateWindow(width, height, title, NULL, NULL);
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