/// 这里声明着与渲染器进行互操作所需的所有数据结构

#pragma once

#include "../common/log.h"
#include "../common/mathf/mathf.h"
#include "vulkan/vulkan.h"

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
///
/// @param binding 
/// @param pAttributeCount 
/// @param pAttributeDescriptions 
void get_vertex_data_input_attribute_descriptions(
    uint32_t                            binding,
    uint32_t*                           pAttributeCount,
    VkVertexInputAttributeDescription*  pAttributeDescriptions
);

/// @brief 对应 vkCmdDraw() 的参数
typedef struct DrawItemInfo {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
} DrawItemInfo;

/// 在这里要说明的是，因为管线本身就是高度需求定制化的，
/// 所以以下的 VertexBufferInfo、PipelineDrawInfo 和 PipelinesDrawInfo 等结构体
/// 它们都不得不含有指名其所属渲染通道和对应渲染管线的前缀；
/// 定制化体现在它们所含的字段成员数量等，比如 VkBuffer 数、VkImage 数都不尽相同.

/**** MainRenderPass, Triangle 管线 ****/

/// @brief 对应 vkCmdBindVertexBuffers() 的参数
typedef struct MainRenderPassTrianglePipelineVertexBufferInfo {
    // TODO: Buffer 属于用户申请的资源，需用户自己管理其生命周期，所以直接让C#端持有句柄传入即可
    VkBuffer        buffer;
    uint64_t        offset;
    uint32_t        drawItemCount;
    DrawItemInfo*   pDrawItemInfos;
} MainRenderPassTrianglePipelineVertexBufferInfo;

typedef struct MainRenderPassTrianglePipelineDrawInfo {
    MainRenderPassTrianglePipelineVertexBufferInfo vertexBufferInfo;
} MainRenderPassTrianglePipelineDrawInfo;

typedef struct MainRenderPassPipelinesDrawInfo {
    MainRenderPassTrianglePipelineDrawInfo triangle;
    
} MainRenderPassPipelinesDrawInfo;



