#pragma once

#include "../../vulkan/vma/vk_mem_alloc.h"

#include "renderer_context_deletion_list_base.h"

typedef struct RendererContext RendererContext;

typedef struct RctxAllocationDeletionList {
    RctxDeletionListBase    base;
    VmaAllocation*          allocations;
} RctxAllocationDeletionList;

void allocation_deletion_list_init(RctxAllocationDeletionList* pList, uint32_t capacity);

void allocation_deletion_list_release(
    RendererContext*            pContext,
    RctxAllocationDeletionList* pList
);