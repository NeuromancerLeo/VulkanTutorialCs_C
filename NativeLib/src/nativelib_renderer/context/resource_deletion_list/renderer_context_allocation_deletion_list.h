#pragma once

#include "renderer_context_deletion_list_base.h"
#include "../../vulkan/vma/vk_mem_alloc.h"

#include <stdlib.h>

typedef struct RctxAllocationDeletionList {
    RctxDeletionListBase    base;
    VmaAllocation*          allocations;
} RctxAllocationDeletionList;

void allocation_deletion_list_init(RctxAllocationDeletionList* pList, uint32_t capacity);

void allocation_deletion_list_release(
    RendererContext*            pContext,
    RctxAllocationDeletionList* pList
);