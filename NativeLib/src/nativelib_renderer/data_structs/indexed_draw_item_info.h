#pragma once

#include <stdint.h>

/// @brief 逻辑上的索引式绘制物体（Indexed Draw Item）信息，内部字段对应着
/// vkCmdDrawIndexed() 的参数，在 BindingBuffersInfo 中提供多个该结构体会使得
/// rendererDrawFrame() 内部在对应管线绑定一次对应 Buffers 后进行多次 Draw Call（绘制调用），
/// 以达到绘制多个物体的效果.
typedef struct IndexedDrawItemInfo {

    /// @brief 要绘制的索引（顶点）数.
    uint32_t indexCount;

    /// @brief 实例化绘制数.
    uint32_t instanceCount;

    /// @brief 要绘制的第一个索引在索引缓冲区中的索引.
    uint32_t firstIndex;

    /// @brief 索引到顶点要后应用的顶点偏移量.
    int32_t  vertexOffset;
    
    /// @brief 实例化绘制偏移量.
    uint32_t firstInstance;

    /// @brief 其 drawItemUniform 在 DrawItems 描述符集中的描述符 Buffer 中的动态偏移量.
    uint32_t uniformDescBufferDynamicOffset;

} IndexedDrawItemInfo;