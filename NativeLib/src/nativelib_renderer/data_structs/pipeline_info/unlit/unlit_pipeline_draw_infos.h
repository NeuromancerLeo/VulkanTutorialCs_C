#pragma once

#include <vulkan/vulkan.h>

typedef struct DrawItemInfo DrawItemInfo;
typedef struct IndexedDrawItemInfo IndexedDrawItemInfo;
typedef struct UnlitMaterialDrawInfo UnlitMaterialDrawInfo;

// 一个 MaterialDrawInfo 表示一种材质实例，所以这里存放材质实例的具体参数；
// 有多个物体使用该种材质实例，所以把这样的多个物体的绘制信息放到同一个
// MaterialDrawInfo 下，表示使用该材质实例的所有物体的绘制信息.
// 注：这里说的是材质实例，不是材质种类（材质种类与管线相关，这里是 Unlit）

/// @brief 表示 Unlit 材质类型下一个需绘制的材质实例.
typedef struct UnlitMaterialDrawInfo {

    /// @brief 该材质实例的 Material 描述符集中，materialUniform 描述符的 Buffer 的大小
    ///（为创建 Buffer 时申请的大小！内部用于计算当前飞行帧对应的偏移）
    size_t                  uniformDescBufferSize;

    /// @brief 该材质实例的 Material 描述符集.
    VkDescriptorSet         descSet;

    /// @brief 该材质实例下的绘制物体信息（DrawItemInfo）数量.
    uint32_t                drawItemCount;

    /// @brief 绘制物体信息数组.
    DrawItemInfo*           pDrawItemInfos;

    /// @brief 该材质实例下的索引式绘制物体信息（IndexedDrawItem）数量.
    uint32_t                indexedDrawItemCount;

    /// @brief 索引式绘制物体信息数组.
    IndexedDrawItemInfo*    pIndexedDrawItemInfos;

} UnlitMaterialDrawInfo;

/// @brief Unlit 管线绘制所需的绑定绘制信息.
typedef struct UnlitPipelineBindingDrawInfo {

    /// @brief 绘制所需绑定的顶点缓冲区.
    VkBuffer                vertexBuffer;

    /// @brief 绑定 vertexBuffer 时要应用的偏移量（字节）.
    uint64_t                vertexOffset;

    /// @brief 绘制所需绑定的索引缓冲区.
    VkBuffer                indexBuffer;

    /// @brief 绑定 indexBuffer 时要应用的偏移量（字节）.
    uint64_t                indexOffset;

    /// @brief 绘制所需绑定的 DrawItems 描述符集中，drawItemUniform 描述符的 Buffer 的大小
    ///（为创建 Buffer 时申请的大小！内部用于计算当前飞行帧对应的偏移）
    size_t                  drawItemUniformDescBufferSize;

    /// @brief 绘制所需绑定的 DrawItems 描述符集.
    VkDescriptorSet         drawItemsDescSet;

    /// @brief 为绘制所提供的材质绘制信息（MaterialDrawInfo）数量.
    uint32_t                materialDrawCount;

    /// @brief Unlit 材质绘制信息数组.
    UnlitMaterialDrawInfo*  materialDrawInfos;

} UnlitPipelineBindingDrawInfo;

/// @brief 该结构体用于填写 Unlit 管线进行工作时所需的所有信息.
typedef struct UnlitPipelineDrawInfo {

    /// @brief 绑定的相机描述符集其 UniformBuffer 的大小
    ///（为创建 Buffer 时申请的大小！内部用于计算当前飞行帧对应的偏移）
    size_t                              cameraUniformDescBufferSize;

    /// @brief 绑定相机描述符集时其 cameraUniform 描述符的 Buffer 要应用的动态偏移量
    uint64_t                            cameraUniformDescBufferDynamicOffset;

    /// @brief 要绑定的相机描述符集
    VkDescriptorSet                     cameraDescSet;

    /// @brief 该字段指示要传入多少个绑定绘制信息以在管线下进行多次绑定与绘制.
    ///
    /// 也对应着调用 vkCmdBindVertexBuffers()、vkCmdBindIndexBuffer() 的次数.
    ///
    /// 多次的绑定会造成大量的状态切换，非常影响性能，请尽量减少传入的 BindingDrawInfo.
    uint32_t                            bindingCount;

    /// @brief 为 Unlit 管线绘制提供的绑定绘制信息数组.
    UnlitPipelineBindingDrawInfo*       pBindingDrawInfos;

} UnlitPipelineDrawInfo;