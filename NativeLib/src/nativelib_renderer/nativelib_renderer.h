#pragma once

#include <stdbool.h>

#include <vulkan/vulkan.h>
#include "vulkan/vma/vk_mem_alloc.h"
#include <GLFW/glfw3.h>

#include "../common/nativelib.h"

typedef struct MainRenderPassDrawInfo MainRenderPassDrawInfo;


EX_API bool rendererInitialize(GLFWwindow* window);


/// @deprecated 请在后期删除该函数
EX_API bool rendererReady();


EX_API bool rendererCreateStaticVertexBuffer(
    size_t              dataSize,
    const void*         pVertexData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


EX_API bool rendererCreateStaticIndexBuffer(
    uint32_t            dataSize,
    const uint32_t*     pIndexData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


/// @brief 获取渲染器对 Uniform 缓冲区所规定的最小偏移量对齐要求值，当你构建一个用于传入
/// Uniform 变量的动态缓冲区时，你必须负责保证缓冲区内的 Uniform 单元数据结构的大小（以及缓冲
/// 区的总大小）满足该函数返回值的整倍数，否则渲染器管线将无法通过你在绘制信息中传入的动态偏移量
/// （Dynamic Offset）来正确读取和使用 Uniform 变量.
///
/// @return 渲染器对 Uniform 缓冲区所规定的最小偏移量对齐要求值
EX_API uint32_t rendererGetMinimalUniformBufferOffsetAlignment();


EX_API bool rendererCreateDynamicUniformBuffer(
    size_t              bufferSize,
    uint32_t            dataOffset,
    size_t              dataSize,
    const void*         pUniformData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


EX_API void rendererUpdateUniformBuffer(
    size_t          bufferSize,
    uint32_t        dataOffset,
    size_t          dataSize,
    void*           pData,
    VmaAllocation   allocation
);


EX_API void rendererRequestDestroyBuffer(VkBuffer buffer, VmaAllocation allocation);


EX_API void rendererDestroyBuffer(VkBuffer buffer, VmaAllocation allocation);


EX_API void rendererBeginFrame();


EX_API void rendererDrawFrame(MainRenderPassDrawInfo *pMainRenderPassDrawInfo);


EX_API void rendererEndFrame();


EX_API void rendererRelease();