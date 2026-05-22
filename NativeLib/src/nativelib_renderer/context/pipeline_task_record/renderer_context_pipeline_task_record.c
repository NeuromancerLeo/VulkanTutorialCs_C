#include "renderer_context_pipeline_task_record.h"

#include "../../../common/log.h"
#include "../structs/renderer_context_structs.h"
#include "../../data_structs/renderer_data_structs.h"


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
    UnlitPipelineDrawInfo* pDrawInfo = (UnlitPipelineDrawInfo*)pPipelineDrawInfo;

    // 绑定 Unlit 管线
    vkCmdBindPipeline(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pContext->mainRenderPassPipelines.unlit.pipeline);

    // 处理管线的动态状态
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

    // 绑定 Camera 描述符集
    // 计算当前飞行帧下实际的偏移量
    uint32_t cameraUniformDescOffset =
        (pContext->currentFrameInFlightIndex * pDrawInfo->cameraUniformDescBufferSize)
        + pDrawInfo->cameraUniformDescBufferDynamicOffset;
    // 绑定
    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pContext->mainRenderPassPipelines.unlit.pipelineLayout,
        0,
        1,
        &pDrawInfo->cameraDescSet,
        1,
        &cameraUniformDescOffset);

    // 按 pDrawInfo->bindingCount 进行管线的顶点相关绑定
    for (int i = 0; i < pDrawInfo->bindingCount; i++)
    {
        // 绑定顶点缓冲区
        vkCmdBindVertexBuffers(commandBuffer,
            // 起始绑定索引，管线规定的，不得乱写
            pContext->mainRenderPassPipelines.unlit.vertexFirstBindingIndex,
            // 绑定数 (起始往下)，不得乱写
            pContext->mainRenderPassPipelines.unlit.vertexBindingCount,
            // Buffer 数组，个数对应绑定数，也不得乱写
            &pDrawInfo->pBindingDrawInfos[i].vertexBuffer,
            // Buffer 数组的 offset 数组，个数对应绑定数，也不得乱写
            &pDrawInfo->pBindingDrawInfos[i].vertexOffset);

        // 绑定索引
        vkCmdBindIndexBuffer(commandBuffer,
            pDrawInfo->pBindingDrawInfos[i].indexBuffer,
            pDrawInfo->pBindingDrawInfos[i].indexOffset,
            VK_INDEX_TYPE_UINT32);

        // 避免过长调用链
        UnlitPipelineBindingDrawInfo* pBindingDrawInfo =
            &pDrawInfo->pBindingDrawInfos[i];

        /**** Unlit 管线的 Draw Call (by Materials) ****/
        // 按 pDrawInfo->pBindingDrawInfos[i].materialDrawCount 进行材质描述符绑定
        for (int j = 0; j < pBindingDrawInfo->materialDrawCount; j++)
        {
            // // 绑定 Material 描述符集
            // // 计算当前飞行帧下实际的偏移量，材质的描述符 Buffer 不使用动态偏移，仅使用副本
            // // 偏移，所以这里只应用当前飞行帧索引下的副本偏移量
            // uint32_t materialUniformDescOffset =
            //     (pContext->currentFrameInFlightIndex
            //     * pBindingDrawInfo->materialDrawInfos[j]->uniformDescBufferSize);
            // vkCmdBindDescriptorSets(commandBuffer,
            //     VK_PIPELINE_BIND_POINT_GRAPHICS,
            //     pContext->mainRenderPassPipelines.unlit.pipelineLayout,
            //     1,
            //     1,
            //     &pBindingDrawInfo->materialDrawInfos[j].descSet,
            //     1,
            //     &materialUniformDescOffset);

            // 避免过长调用链
            UnlitMaterialDrawInfo* pMaterialDrawInfo =
                &pBindingDrawInfo->materialDrawInfos[j];

            // 开始 Draw Call

            // 避免过长调用链
            DrawItemInfo* pDrawItemInfo = NULL;
            IndexedDrawItemInfo* pIndexedDrawItemInfo = NULL;

            uint32_t drawItemUniformDescOffset = 0;
            int k = 0;
            // 按 materialDrawInfos[j].drawItemCount 次调用 vkCmdDraw()
            for (; k < pMaterialDrawInfo->drawItemCount; k++)
            {
                pDrawItemInfo = &pMaterialDrawInfo->pDrawItemInfos[k];

                // 绑定 DrawItems 描述符集（应用当前 drawItem 的动态偏移量）
                // 计算当前飞行帧下的实际偏移量
                drawItemUniformDescOffset =
                    (pContext->currentFrameInFlightIndex
                    * pBindingDrawInfo->drawItemUniformDescBufferSize)
                    + pDrawItemInfo->uniformDescBufferDynamicOffset;
                vkCmdBindDescriptorSets(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pContext->mainRenderPassPipelines.unlit.pipelineLayout,
                    1,  // TODO: set 编号暂时为 1，因为我还没实现 material set
                    1,
                    &pBindingDrawInfo->drawItemsDescSet,
                    1,
                    &drawItemUniformDescOffset);

                vkCmdDraw(commandBuffer,
                    pDrawItemInfo->vertexCount,
                    pDrawItemInfo->instanceCount,
                    pDrawItemInfo->firstVertex,
                    pDrawItemInfo->firstInstance);
            }

            // 按 materialDrawInfos[j].indexedDrawItemCount 次调用 vkCmdDrawIndexed()
            for (k = 0; k < pMaterialDrawInfo->indexedDrawItemCount; k++)
            {
                pIndexedDrawItemInfo = &pMaterialDrawInfo->pIndexedDrawItemInfos[k];

                // 绑定 DrawItems 描述符集（应用当前 indexedDrawItem 的动态偏移量）
                // 计算当前飞行帧下的实际偏移量
                drawItemUniformDescOffset =
                    (pContext->currentFrameInFlightIndex
                    * pBindingDrawInfo->drawItemUniformDescBufferSize)
                    + pIndexedDrawItemInfo->uniformDescBufferDynamicOffset;
                vkCmdBindDescriptorSets(commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pContext->mainRenderPassPipelines.unlit.pipelineLayout,
                    1,  // TODO: set 编号暂时为 1，因为我还没实现 material set
                    1,
                    &pBindingDrawInfo->drawItemsDescSet,
                    1,
                    &drawItemUniformDescOffset);
    
                vkCmdDrawIndexed(commandBuffer,
                    pIndexedDrawItemInfo->indexCount,
                    pIndexedDrawItemInfo->instanceCount,
                    pIndexedDrawItemInfo->firstIndex,
                    pIndexedDrawItemInfo->vertexOffset,
                    pIndexedDrawItemInfo->firstInstance);
            }

        }  // for each materialDrawInfo

    }  // for each bindingDrawInfo

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