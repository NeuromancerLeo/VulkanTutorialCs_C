#pragma once

#include "../../context/structs/renderer_context_structs.h"

/// @brief 一个该信息结构体代表了渲染通道实例中一个 subpass 中的一个特定管线的绘制任务.
typedef struct PipelineDrawTask {

    /// @brief 指明该绘制任务针对的是渲染器上下文中定义的哪一个管线.
    RctxPipelineType    pipelineType;

    /// @brief 针对该绘制任务的目标管线进行绘制命令录制的函数的指针，由系统自动填充.
    ///
    /// 内部 DrawFrame() 处理该绘制任务时自动调用，作为调用者你无需也不应填写该字段.
    ///
    /// 在外部定义该结构体时，应该将该字段作为保留字段（8 byte）不作处理.
    void                (*record)(
                            RendererContext*    pContext,
                            VkCommandBuffer     commandBuffer,
                            void*               pipelineDrawInfo
                        );

    /// @brief 绘制任务的目标管线所需的具体绘制信息.
    void*               pPipelineDrawInfo;

} PipelineDrawTask;