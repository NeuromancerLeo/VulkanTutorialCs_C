#include "renderer_context_buffer_deletion_list.h"

#include "../structs/renderer_context_structs.h"

static bool buffer_deletion_list_add(
    RctxDeletionListBase*   pBase,
    void*                   resource
)
{
    VkBuffer buffer = (VkBuffer)resource;
    RctxBufferDeletionList* list = (RctxBufferDeletionList*)pBase;

    if (list->base.capacity == 0)
    {
        log_error("%s(): 返回，list->base.capacity == 0，你是否忘记调用相关 init 函数？",
            __func__);

        return false;
    }

    if (list->base.count >= list->base.capacity)
    {
        uint32_t newCapacity = list->base.capacity * 2;

        VkBuffer* newBuffers =
            (VkBuffer*)realloc(list->buffers,
                sizeof(VkBuffer) * newCapacity);
        if (!newBuffers)
        {
            log_error("%s(): realloc 失败！", __func__);

            return false;
        }

        list->buffers   = newBuffers;
        list->base.capacity = newCapacity;
    }

    list->buffers[list->base.count] = buffer;
    list->base.count ++;

    return true;
}

static void buffer_deletion_list_flush(
    RendererContext*        pContext,
    RctxDeletionListBase*   pBase
)
{
    RctxBufferDeletionList* list = (RctxBufferDeletionList*)pBase;
    
    for (int i = 0; i < list->base.count; i++)
    {
        vwrpDestroyBuffer(pContext->device, list->buffers[i]);
    }

    list->base.count = 0;
}

static const RctxDeletionListOps buffer_deletion_list_ops = {
    .add    = buffer_deletion_list_add,
    .flush  = buffer_deletion_list_flush
};


void buffer_deletion_list_init(RctxBufferDeletionList* pList, uint32_t capacity)
{
    if (capacity == 0)
        capacity = 64;

    pList->base.capacity    = capacity;
    pList->base.count       = 0;
    pList->base.pOps        = &buffer_deletion_list_ops;

    pList->buffers          = (VkBuffer*)calloc(capacity , sizeof(VkBuffer));
}


void buffer_deletion_list_release(
    RendererContext*        pContext,
    RctxBufferDeletionList* pList
)
{
    if (pList->base.count == 0)
    {
        free(pList->buffers);

        return;
    }

    buffer_deletion_list_flush(pContext, &pList->base);
    free(pList->buffers);
}