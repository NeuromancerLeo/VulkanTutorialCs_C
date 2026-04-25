/// 这里声明着与渲染器进行互操作所需的所有数据结构

#pragma once

#include "../common/log.h"
#include "../common/mathf/mathf.h"
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

/// @brief 代表顶点属性的数据结构.
typedef struct VertexData {
    Vector2 position;
    Vector3 color;
} VertexData;

/// @brief 获取用 `VertexData` 数据结构作图形管线的顶点输入（顶点缓冲区）的数据单元时，创建管线
/// 要用到的 VkVertexInputBindingDescription 信息结构体（用于描述图形管线的顶点缓冲区绑定）.
///
/// @param binding 指定一个绑定索引，对应着调用 vkCmdBindVertexBuffers() 时所传入的 VkBuffer
///
/// @return 填写好的 VkVertexInputBindingDescription 信息结构体，其字段根据 `VertexData`
/// 设置 
VkVertexInputBindingDescription get_vertex_data_input_binding_description(
    uint32_t binding
);

/// @brief 获取用 `VertexData` 数据结构作图形管线的顶点输入（顶点缓冲区）的数据单元时，创建管线
/// 要用到的 VkVertexInputAttributeDescription 信息结构体（用于顶点缓冲区中具体的属性的描述）.
///
///（和 Vulkan 查询 API 类似，先调用一次该函数获取属性描述结构体的数量，然后再调用一次传入数组供
/// 函数填充）
void get_vertex_data_input_attribute_descriptions(
    uint32_t                            binding,
    uint32_t*                           pAttributeCount,
    VkVertexInputAttributeDescription*  pAttributeDescriptions
);

/// @brief 逻辑上的绘制物体（Draw Item）信息，内部字段对应着 vkCmdDraw() 的参数，在
/// BindingBuffersInfo 中提供多个该结构体会使得 rendererDrawFrame() 内部在对应管线绑定一次对
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

} DrawItemInfo;

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

} IndexedDrawItemInfo;

/// 在这里要说明的是，因为管线本身就是高度需求定制化的，
/// 所以以下的 BindingBuffersInfo、PipelineDrawInfo 和 PipelinesDrawInfo 等结构体
/// 它们都不得不含有指明其所属渲染管线或对应渲染通道的前缀；
/// 定制化体现在它们所含的字段成员数量等，比如 VkBuffer 数、VkImage 数都不尽相同.

/**** MainRenderPass, Triangle 管线 ****/

/// @brief Triangle 管线要绑定的缓冲区集信息.
typedef struct TrianglePipelineBindingBuffersInfo {

    /// @brief Triangle 管线绘制所需的顶点缓冲区.
    VkBuffer                vertexBuffer;

    /// @brief 绑定 vertexBuffer 时要应用的偏移量（字节）.
    uint64_t                vertexOffset;

    /// @brief Triangle 管线绘制所需的索引缓冲区.
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

} TrianglePipelineBindingBuffersInfo;

/// @brief 该结构体用于填写 Triangle 管线进行工作时所需的所有信息.
typedef struct TrianglePipelineDrawInfo {

    /// @brief 该字段指示要传入多少个 Buffer 集以在管线下进行多次绑定与多次绘制，即对应着调用
    /// vkCmdBindVertexBuffers() 的次数.
    ///
    /// 多次的绑定会造成大量的状态切换，非常影响性能，请尽量减少传入的 BindingBuffersInfo.
    uint32_t                                buffersBindingCount;

    /// @brief Triangle 管线要绑定的 Buffer 集信息数组.
    TrianglePipelineBindingBuffersInfo*     pBindingBuffersInfos;

} TrianglePipelineDrawInfo;

/**** MainRenderPass ****/

typedef struct MainRenderPassPipelinesDrawInfo {
    TrianglePipelineDrawInfo triangle;
    // ...
} MainRenderPassPipelinesDrawInfo;




