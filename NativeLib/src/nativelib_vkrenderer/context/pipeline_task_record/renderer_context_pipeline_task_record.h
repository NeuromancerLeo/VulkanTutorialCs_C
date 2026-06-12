#pragma once

#include <vulkan/vulkan.h>

typedef struct RendererContext RendererContext;
typedef struct MainRenderPassDrawInfo MainRenderPassDrawInfo;


void pipelineTaskRecordTriangle(
    RendererContext*    pContext,
    VkCommandBuffer     commandBuffer,
    void*               pPipelineDrawInfo
);


void pipelineTaskRecordUnlit(
    RendererContext*    pContext,
    VkCommandBuffer     commandBuffer,
    void*               pDrawInfo
);


/// @brief 设置传入的 RenderPassDrawInfo 内所有 PipelineDrawTask 对应的 record 函数指针.
void set_pipeline_task_record_func(
    MainRenderPassDrawInfo* pMainRenderPassDrawInfo
);
