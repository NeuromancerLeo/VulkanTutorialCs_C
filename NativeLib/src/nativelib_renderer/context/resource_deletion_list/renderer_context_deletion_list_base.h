#pragma once

#include <stdint.h>

#include "../../../common/log.h"

// 前向声明
typedef struct RendererContext RendererContext;

typedef struct RctxDeletionListBase RctxDeletionListBase;

typedef struct RctxDeletionListOps {
    // 必须
    bool (*add)(RctxDeletionListBase* pBase, void* resource);
    // 必须
    void (*flush)(RendererContext* pContext, RctxDeletionListBase* pBase);
} RctxDeletionListOps;

struct RctxDeletionListBase {
    uint32_t                    capacity;
    uint32_t                    count;
    const RctxDeletionListOps*  pOps;   // 操作表
};

bool deletion_list_add(RctxDeletionListBase* pBase, void* resource);

void deletion_list_flush(RendererContext* pContext, RctxDeletionListBase* pBase);