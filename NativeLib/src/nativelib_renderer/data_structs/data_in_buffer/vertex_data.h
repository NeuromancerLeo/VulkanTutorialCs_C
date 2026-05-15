#pragma once

#include <vulkan/vulkan.h>

#include "../../../common/mathf/mathf.h"

/// @brief 代表顶点属性的数据结构.
typedef struct VertexData {
    Vector3     position;
    float       padding0;

    Vector3     color;
    float       padding1;
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