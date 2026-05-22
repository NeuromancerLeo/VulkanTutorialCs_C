#include "renderer_context_deletion_list_base.h"

#include "../../../common/log.h"


bool deletion_list_add(
    RctxDeletionListBase*   pBase,
    void*                   resource
)
{
    if (!pBase->pOps->add)
    {
        log_error("%s(): pBase->pOps->add() 为空！", __func__);

        return false;
    }

    return pBase->pOps->add(pBase, resource);
}


void deletion_list_flush(RendererContext* pContext, RctxDeletionListBase* pBase)
{
    if (!pBase->pOps->flush)
    {
        log_error("%s(): pBase->pOps->flush() 为空！", __func__);

        return;
    }

    pBase->pOps->flush(pContext, pBase);
}