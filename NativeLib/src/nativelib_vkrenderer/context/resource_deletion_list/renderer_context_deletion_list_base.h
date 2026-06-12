#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

typedef struct RendererContext RendererContext;
typedef struct RctxDeletionListBase RctxDeletionListBase;

typedef struct RctxDeletionListOps {
    // 必须
    bool (*add)(RctxDeletionListBase* pBase, void* resource);
    // 必须
    void (*flush)(RendererContext* pContext, RctxDeletionListBase* pBase);
} RctxDeletionListOps;

struct RctxDeletionListBase {
    bool                        initialized;
    pthread_mutex_t*            pMutex;
    uint32_t                    capacity;
    uint32_t                    count;
    const RctxDeletionListOps*  pOps;   // 操作表
};


/// @brief 添加资源句柄到指定的删除列表中去.
///
/// @note ThreadSafe: 该函数自动带锁，保证线程安全.
///
/// @param pBase 指定的删除列表 Base
/// @param resource 目标资源句柄
///
/// @return 添加失败返回 `false`，成功则返回 `true`
bool deletion_list_add(RctxDeletionListBase* pBase, void* resource);


/// @brief 刷新指定的删除列表，释放表中记录的所有资源句柄并从表中移除它们.
///
/// @note ThreadSafe: 该函数自动带锁，保证线程安全.
///
/// @param pContext 调用该函数需要传入有效的渲染器上下文
/// @param pBase 指定的删除列表 Base
void deletion_list_flush(RendererContext* pContext, RctxDeletionListBase* pBase);