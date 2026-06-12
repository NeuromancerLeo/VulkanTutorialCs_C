#include "renderer_context_deletion_list_base.h"

#include "../../../common/log.h"


bool deletion_list_add(
    RctxDeletionListBase    *pBase,
    void                    *resource
)
{
    pthread_mutex_lock(pBase->pMutex);

    bool isSuccess = pBase->pOps->add(pBase, resource);

    pthread_mutex_unlock(pBase->pMutex);

    return isSuccess;
}


void deletion_list_flush(RendererContext* pContext, RctxDeletionListBase* pBase)
{
    pthread_mutex_lock(pBase->pMutex);

    pBase->pOps->flush(pContext, pBase);

    pthread_mutex_unlock(pBase->pMutex);
}