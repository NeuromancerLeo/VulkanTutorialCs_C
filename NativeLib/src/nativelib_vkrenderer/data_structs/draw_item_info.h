#pragma once

#include <stdint.h>

/// @brief 逻辑上的绘制物体（Draw Item）信息，内部字段对应着 vkCmdDraw() 的参数，在
/// BindingBuffersInfo 中提供多个该结构体会使得 vrdrDrawFrame() 内部在对应管线绑定一次对
/// 应 Buffers 后进行多次 Draw Call（绘制调用），以达到绘制多个物体的效果.
typedef struct DrawItemInfo {

    /// @brief 要绘制的顶点数.
    uint32_t vertexCount;

    /// @brief 实例化绘制数.
    uint32_t instanceCount;
    
    /// @brief 要绘制的第一个顶点在顶点缓冲区中的索引.
    uint32_t firstVertex;
    
    /// @brief 实例化绘制偏移量.
    uint32_t firstInstance;

    /// @brief 该绘制物体的 `DrawItemUniform` 在 DrawItem 描述符集的描述符
    /// DrawItemUniformBuffer 中的动态偏移量.
    uint32_t uniformBufferDescDynamicOffset;

} DrawItemInfo;