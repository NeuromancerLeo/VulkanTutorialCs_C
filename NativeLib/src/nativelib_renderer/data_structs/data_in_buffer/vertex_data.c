#include "vertex_data.h"


VkVertexInputBindingDescription get_vertex_data_input_binding_description(
    uint32_t binding
)
{
    VkVertexInputBindingDescription vertexInputBindingDescription = {};
    // 绑定索引值对应着调用 vkCmdBindVertexBuffers() 时所传入的 VkBuffer，不同的绑定值意为
    // 不同的数据源（也就是不同的 VkBuffer），由于顶点着色器中对顶点属性的读取只引用其
    // location，所以描述顶点属性时，即下面的
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
    uint32_t*                           outAttributeCount,
    VkVertexInputAttributeDescription*  outAttributeDescriptions
)
{
    if (!outAttributeCount)
    {
        log_error("%s()：无效的参数，传入的 outAttributeCount 不得为 NULL！", __func__);

        return;
    }

    if (!outAttributeDescriptions)
    {
        *outAttributeCount = 2;
        return;
    }

    outAttributeDescriptions[0].binding  = binding;
    outAttributeDescriptions[0].location = 0;
    outAttributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    outAttributeDescriptions[0].offset   = offsetof(VertexData, position);

    outAttributeDescriptions[1].binding  = binding;
    outAttributeDescriptions[1].location = 1;
    outAttributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    outAttributeDescriptions[1].offset   = offsetof(VertexData, color);

    *outAttributeCount = 2;
}