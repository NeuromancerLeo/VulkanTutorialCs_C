#include "renderer_context_allocation_deletion_list.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "../../../common/log.h"
#include "renderer_context_deletion_list_base.h"
#include "../structs/renderer_context_structs.h"

static bool allocation_deletion_list_add(
    RctxDeletionListBase*       pBase,
    void*                       resource
)
{
    VmaAllocation allocation = (VmaAllocation)resource;
    RctxAllocationDeletionList* list = (RctxAllocationDeletionList*)pBase;

#ifdef DEBUG
    if (list->base.capacity == 0)
    {
        log_error("%s(): 返回，list->base.capacity = 0，你是否忘记调用相关 init 函数？",
            __func__);

        return false;
    }

    if (!list->base.initialized)
    {
        log_error("%s(): 返回，list->base.initialized = false，请先调用相关 init 函数！",
            __func__);

        return false;
    }
#endif

    if (list->base.count >= list->base.capacity)
    {
        uint32_t newCapacity = list->base.capacity * 2;

        VmaAllocation* newAllocations =
            (VmaAllocation*)realloc(list->allocations,
                sizeof(VmaAllocation) * newCapacity);
        if (!newAllocations)
        {
            log_error("%s(): realloc 失败！", __func__);

            return false;
        }

        list->allocations   = newAllocations;
        list->base.capacity = newCapacity;
    }

    list->allocations[list->base.count] = allocation;
    list->base.count ++;

    return true;
}

static void allocation_deletion_list_flush(
    RendererContext*            pContext,
    RctxDeletionListBase*       pBase
)
{
#ifdef DEBUG
    if (!list->base.initialized)
    {
        log_error("%s(): 返回，list->base.initialized = false，请先调用相关 init 函数！",
            __func__);

        return false;
    }
#endif

    RctxAllocationDeletionList* list = (RctxAllocationDeletionList*)pBase;
    
    for (int i = 0; i < list->base.count; i++)
    {
        vmaFreeMemory(pContext->vmaAllocator, list->allocations[i]);
    }

    list->base.count = 0;
}

static const RctxDeletionListOps allocation_deletion_list_ops = {
    .add    = allocation_deletion_list_add,
    .flush  = allocation_deletion_list_flush
};


void allocation_deletion_list_init(RctxAllocationDeletionList* pList, uint32_t capacity)
{
    if (pList->base.initialized)
        return;

    pthread_mutex_init(pList->base.pMutex, NULL); // 初始化锁

    if (capacity == 0)
        capacity = 64;

    pList->base.capacity    = capacity;
    pList->base.count       = 0;
    pList->base.pOps        = &allocation_deletion_list_ops;

    pList->allocations      = (VmaAllocation*)calloc(capacity , sizeof(VmaAllocation));

    pList->base.initialized = true;
}


void allocation_deletion_list_release(
    RendererContext*            pContext,
    RctxAllocationDeletionList* pList
)
{
    if (!pList->base.initialized)
        return;

    if (!pList->base.count == 0)
        allocation_deletion_list_flush(pContext, &pList->base);
    
    free(pList->allocations);
    
    pthread_mutex_destroy(pList->base.pMutex); // 销毁锁

    pList->base.initialized = false;
}