#pragma once

#include <vulkan/vulkan.h>
#include <stdlib.h>

#include "renderer_context_deletion_list_base.h"
#include "../../vulkan/wrapper/vulkan_wrapper.h"

typedef struct RctxBufferDeletionList {
    RctxDeletionListBase    base;
    VkBuffer*               buffers;
} RctxBufferDeletionList;

void buffer_deletion_list_init(RctxBufferDeletionList* pList, uint32_t capacity);

void buffer_deletion_list_release(
    RendererContext*        pContext,
    RctxBufferDeletionList* pList
);