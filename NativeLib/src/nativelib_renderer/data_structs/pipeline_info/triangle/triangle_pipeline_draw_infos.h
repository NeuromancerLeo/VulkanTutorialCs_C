#pragma once

#include <vulkan/vulkan.h>

#include "../../draw_item_info.h"
#include "../../indexed_draw_item_info.h"

/// @brief Triangle 管线绘制所需的绑定绘制信息.
typedef struct TrianglePipelineBindingDrawInfo {

    /// @brief 绘制所需绑定的顶点缓冲区.
    VkBuffer                vertexBuffer;

    /// @brief 绑定 vertexBuffer 时要应用的偏移量（字节）.
    uint64_t                vertexOffset;

    /// @brief 绘制所需绑定的索引缓冲区.
    VkBuffer                indexBuffer;

    /// @brief 绑定 indexBuffer 时要应用的偏移量（字节）.
    uint64_t                indexOffset;

    /// @brief 提供的绘制物体信息（DrawItemInfo）数量.
    uint32_t                drawItemCount;

    /// @brief 绘制物体信息数组.
    DrawItemInfo*           pDrawItemInfos;

    /// @brief 提供的索引式绘制物体信息（IndexedDrawItem）数量.
    uint32_t                indexedDrawItemCount;

    /// @brief 索引式绘制物体信息数组.
    IndexedDrawItemInfo*    pIndexedDrawItemInfos;

} TrianglePipelineBindingDrawInfo;

/// @brief 该结构体用于填写 Triangle 管线进行工作时所需的所有信息.
typedef struct TrianglePipelineDrawInfo {

    /// @brief 该字段指示要传入多少个绑定绘制信息以在管线下进行多次绑定与绘制.
    ///
    /// 也对应着调用 vkCmdBindVertexBuffers()、vkCmdBindIndexBuffer() 的次数.
    ///
    /// 多次的绑定会造成大量的状态切换，非常影响性能，请尽量减少传入的 BindingDrawInfo.
    uint32_t                            bindingCount;

    /// @brief 为 Unlit 管线绘制提供的绑定绘制信息数组.
    TrianglePipelineBindingDrawInfo*    pBindingDrawInfos;

} TrianglePipelineDrawInfo;