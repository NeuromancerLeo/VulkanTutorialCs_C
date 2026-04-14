#pragma once

#include "../common/nativelib.h"
#include "renderer_context.h"
#include "renderer_data_structs.h"

#include <stdbool.h>
#include <GLFW/glfw3.h>


EX_API bool rendererInitialize(GLFWwindow* window);


EX_API bool rendererReady();


EX_API BufferResource rendererCreateStaticVertexBuffer(
    size_t              dataSize,
    const VertexData*   pVertiesData
);


EX_API BufferResource rendererCreateStaticIndexBuffer(
    size_t              dataSize,
    const IndexData*    pIndicesData
);


EX_API BufferResource rendererCreateDynamicUniformBuffer();


EX_API BufferResource rendererUpdateUniformBuffer();


EX_API void rendererDestroyBuffer();


EX_API void rendererDrawFrame();


EX_API void rendererRelease();