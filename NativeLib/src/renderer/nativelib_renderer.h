#pragma once

#include "../common/log.h"
#include "../common/nativelib.h"
#include "renderer_context.h"
#include "renderer_data_structs.h"

#include <stdbool.h>
#include <GLFW/glfw3.h>


EX_API bool rendererInitialize(GLFWwindow* window);


EX_API bool rendererReady();


EX_API bool rendererCreateStaticVertexBuffer(
    uint32_t            dataSize,
    const VertexData*   pVertiesData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


EX_API bool rendererCreateStaticIndexBuffer(
    uint32_t            dataSize,
    const uint32_t*     pIndicesData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


EX_API bool rendererCreateDynamicUniformBuffer();


EX_API bool rendererUpdateUniformBuffer();


EX_API void rendererDestroyBuffer();


EX_API void rendererDrawFrame();


EX_API void rendererRelease();