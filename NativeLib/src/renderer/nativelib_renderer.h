#pragma once

#include "../common/nativelib.h"
#include "render_context.h"

#include <stdbool.h>
#include <GLFW/glfw3.h>


EX_API bool rendererInitialize(GLFWwindow* window);


EX_API void rendererReady();


EX_API void rendererBeginFrame();


EX_API void rendererEndFrame();


EX_API void rendererRelease();