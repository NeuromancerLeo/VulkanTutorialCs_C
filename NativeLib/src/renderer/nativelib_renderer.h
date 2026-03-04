#pragma once

#include "../common/nativelib.h"
#include "renderer_context.h"
#include "renderer_data_structs.h"

#include <stdbool.h>
#include <GLFW/glfw3.h>


EX_API bool rendererInitialize(GLFWwindow* window);


EX_API bool rendererReady();

// TOOD: VertexBufferHandle rendererCreateVertexBuffer()
//       IndexBufferHandle rendererCreateIndexBuffer()

EX_API void rendererDrawFrame();


EX_API void rendererRelease();