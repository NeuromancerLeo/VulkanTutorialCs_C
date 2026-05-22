#pragma once

#include <stdint.h>

typedef struct PipelineDrawTask PipelineDrawTask;

/// @brief 该结构体表示一个 subpass 所需的绘制信息.
typedef struct SubpassDrawInfo {

    /// @brief 要传入的管线绘制任务数.
    uint32_t            pipelineDrawTaskCount;

    /// @brief 管线绘制任务数组.
    PipelineDrawTask*   pipelineDrawTasks;

} SubpassDrawInfo;

typedef struct MainRenderPassDrawInfo {
    SubpassDrawInfo defaultPass;
    // ...
} MainRenderPassDrawInfo;