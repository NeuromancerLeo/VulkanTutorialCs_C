#include "renderer_data_structs.h"


VkVertexInputBindingDescription get_vertex_data_input_binding_description(
    uint32_t binding
)
{
    VkVertexInputBindingDescription vertexInputBindingDescription = {};
    // 绑定索引值对应着调用 vkCmdBindVertexBuffers() 时所传入的 VkBuffer，不同的绑定值意为
    // 不同的数据源（也就是不同的 VkBuffer），由于顶点着色器中对顶点属性的读取只引用其
    // location，所以描述顶点属性时，如下面的
    // get_vertex_data_input_attribute_descriptions() 函数，无论不同属性其 binding 字段
    // 是否相等，它们的 location 值必须是唯一的
    vertexInputBindingDescription.binding   = binding;
    // 数据单元大小
    vertexInputBindingDescription.stride    = sizeof(VertexData);
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return vertexInputBindingDescription;
}

void get_vertex_data_input_attribute_descriptions(
    uint32_t                            binding,
    uint32_t*                           pAttributeCount,
    VkVertexInputAttributeDescription*  pAttributeDescriptions
)
{
    if (!pAttributeCount)
    {
        log_error("%s()：无效的参数，传入的 pAttributeCount 不得为 NULL！");
        return;
    }

    if (!pAttributeDescriptions)
    {
        *pAttributeCount = 2;
        return;
    }

    pAttributeDescriptions[0].binding  = binding;
    pAttributeDescriptions[0].location = 0;
    pAttributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    pAttributeDescriptions[0].offset   = offsetof(VertexData, position);

    pAttributeDescriptions[1].binding  = binding;
    pAttributeDescriptions[1].location = 1;
    pAttributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[1].offset   = offsetof(VertexData, color);

    *pAttributeCount = 2;
}
