#include "renderer_context_pipeline_task_record.h"


void pipelineTaskRecordTriangle(
    RendererContext*    pContext,
    VkCommandBuffer     commandBuffer,
    void*               pPipelineDrawInfo
)
{
    TrianglePipelineDrawInfo* pDrawInfo = (TrianglePipelineDrawInfo*)pPipelineDrawInfo;

    // 绑定 Triangle 管线
    vkCmdBindPipeline(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pContext->mainRenderPassPipelines.triangle.pipeline);

    // 处理 Triangle 管线的动态状态
    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = pContext->swapchainExtent.width;
    viewport.height     = pContext->swapchainExtent.height;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset   = (VkOffset2D){0, 0};
    scissor.extent   = pContext->swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (int i = 0;
            i < pDrawInfo->bindingCount;
            i++)
    {
        // 为 Triangle 管线绑定顶点缓冲区
        vkCmdBindVertexBuffers(commandBuffer,
            // 起始绑定索引，管线规定的，不得乱写
            pContext->mainRenderPassPipelines.triangle.vertexFirstBindingIndex,
            // 绑定数 (起始往下)，不得乱写
            pContext->mainRenderPassPipelines.triangle.vertexBindingCount,
            // Buffer 数组，个数对应绑定数，也不得乱写
            &pDrawInfo->pBindingDrawInfos[i].vertexBuffer,
            // Buffer 数组的 offset 数组，个数对应绑定数，也不得乱写
            &pDrawInfo->pBindingDrawInfos[i].vertexOffset);

        // 绑定索引
        vkCmdBindIndexBuffer(commandBuffer,
            pDrawInfo->pBindingDrawInfos[i].indexBuffer,
            pDrawInfo->pBindingDrawInfos[i].indexOffset,
            VK_INDEX_TYPE_UINT32);

        // Triangle 管线的 Draw Call

        // 按 PipelineBindingBuffersInfo 中的 drawItemCount 次调用 vkCmdDraw()
        int j = 0;
        for (;
                j < pDrawInfo->pBindingDrawInfos[i].drawItemCount;
                j++)
        {
            vkCmdDraw(commandBuffer,
                pDrawInfo->pBindingDrawInfos[i].pDrawItemInfos[j].vertexCount,
                pDrawInfo->pBindingDrawInfos[i].pDrawItemInfos[j].instanceCount,
                pDrawInfo->pBindingDrawInfos[i].pDrawItemInfos[j].firstVertex,
                pDrawInfo->pBindingDrawInfos[i].pDrawItemInfos[j].firstInstance);
        }

        // 按 PipelineBindingBuffersInfo 中的 indexedDrawItemCount 次调用 vkCmdDrawIndexed()
        for (j = 0;
                j < pDrawInfo->pBindingDrawInfos[i].indexedDrawItemCount;
                j++)
        {
            vkCmdDrawIndexed(commandBuffer,
                pDrawInfo->pBindingDrawInfos[i].pIndexedDrawItemInfos[j].indexCount,
                pDrawInfo->pBindingDrawInfos[i].pIndexedDrawItemInfos[j].instanceCount,
                pDrawInfo->pBindingDrawInfos[i].pIndexedDrawItemInfos[j].firstIndex,
                pDrawInfo->pBindingDrawInfos[i].pIndexedDrawItemInfos[j].vertexOffset,
                pDrawInfo->pBindingDrawInfos[i].pIndexedDrawItemInfos[j].firstInstance);
        }
    }
}


void pipelineTaskRecordUnlit(
    RendererContext*    pContext,
    VkCommandBuffer     commandBuffer,
    void*               pPipelineDrawInfo
)
{

}


void set_pipeline_task_record_func(
    MainRenderPassDrawInfo* pMainRenderPassDrawInfo
)
{
    // Main RenderPass, Default Pass
    for (int i = 0;
         i < pMainRenderPassDrawInfo->defaultPass.pipelineDrawTaskCount;
         i++)
    {
        switch (pMainRenderPassDrawInfo->defaultPass.pipelineDrawTasks[i].pipelineType)
        {
            case RCTX_PIPELINE_TYPE_TRIANGLE:
                pMainRenderPassDrawInfo->defaultPass.pipelineDrawTasks[i].record =
                    pipelineTaskRecordTriangle;
                break;

            case RCTX_PIPELINE_TYPE_UNLIT:
                pMainRenderPassDrawInfo->defaultPass.pipelineDrawTasks[i].record = 
                    pipelineTaskRecordUnlit;
                break;

            default:
                log_error("%s(): 未知的管线类型，请检查渲染器上下文是否定义了对应管线！",
                    __func__);
        }
    }
    // Main RenderPass, ... Pass
    // ...
}