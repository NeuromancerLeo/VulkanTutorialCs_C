#pragma once

#include <stdbool.h>

#include <vulkan/vulkan.h>
#include "../vulkan/vma/vk_mem_alloc.h"

typedef struct RendererContext RendererContext;
typedef VkSurfaceKHR (*SurfaceCreateHelperFunc)(VkInstance);
typedef struct MainRenderPassDrawInfo MainRenderPassDrawInfo;


/// @brief 为一个渲染器上下文分配内存并返回其句柄.
///
/// @return 一个新的 RendererContext 的句柄，发生错误时返回 `NULL`
RendererContext* rctxNewRendererContext();


/// @brief 给定一个渲染器上下文然后对其进行（初步地）初始化构建.
///
/// @param windowExtensionCount 平台窗口所需扩展数
/// @param windowExtensionStrings 平台窗口所需所有扩展的对应字符串
/// @param create_window_surface_helper 平台窗口针对 VkSurface 创建的辅助函数指针
/// @param windowFramebufferWidth 平台窗口帧缓冲区宽度
/// @param windowFramebufferHeight 平台窗口帧缓冲区高度
///
/// @return 当构建成功时返回 `true`，若发生错误则会终止构建（相关函数会输出信息）并返回 `false`
bool rctxCreateRendererContext(
    RendererContext*        pContext,
    uint32_t                windowExtensionCount,
    const char**            windowExtensionStrings,
    SurfaceCreateHelperFunc create_window_surface_helper,
    int                     windowFramebufferWidth,
    int                     windowFramebufferHeight
);


/// @brief 给定渲染器上下文句柄，销毁其（除了窗口句柄外的）所有上下文对象，同时销毁自身释放内存
///
/// @param pContext 要销毁的渲染器上下文句柄
void rctxDestroyRendererContext(RendererContext* pContext);


/// @brief 创建并填充静态性缓冲区，内部使用 StagingBuffer 并使用传送队列复制数据到设备本地的
/// Buffer 上.
///
/// @note ThreadSafe: 该函数不依赖任何运行时状态，已被设计成线程安全的，你可以任意使用不同的线程
/// 来调用该函数.
/// 
/// @param usage 指示缓冲区用途
/// @param bufferSize 用户期望的缓冲区大小
/// @param dataOffset 上传数据时应用的偏移量
/// @param dataSize 上传数据的大小
/// @param pData 要上传的数据
/// @param outBuffer 输出变量，返回 VkBuffer 句柄，发生错误时为 `NULL`
/// @param outAllocation 输出变量，返回 VkBuffer 对应的 VmaAllocaiton 句柄，发生错误时为
/// `NULL`
///
/// @return 函数成功返回 `true`, 发生错误时返回 `false`
bool rctxCreateStaticBuffer(
    RendererContext*        pContext,
    VkBufferUsageFlagBits   usage,
    size_t                  bufferSize,
    uint64_t                dataOffset,
    size_t                  dataSize,
    const void*             pData,
    VkBuffer*               outBuffer,
    VmaAllocation*          outAllocation
);


/// @brief 创建并填充动态性缓冲区，内部使用 Ring Buffer 来解决不同飞行帧状态之间的数据同步安全.
///
/// @warning ThreadUnSafe: 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会
/// 调用 rctxEndFrame() 改变渲染器的当前飞行帧状态，那么该函数就是线程安全的.
///
/// @param usage 指示缓冲区用途
/// @param bufferSize 用户期望的缓冲区大小，其必须是
/// pContext->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment 返回的值的
/// 整数倍
/// @param dataOffset 上传数据时应用的偏移量，其必须是
/// pContext->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment 返回的值的
/// 整数倍
/// @param dataSize 上传数据的大小
/// @param pData 要上传的数据
/// @param outBuffer 输出变量，返回 VkBuffer 句柄，发生错误时为 `NULL`
/// @param outAllocation 输出变量，返回 VkBuffer 对应的 VmaAllocaiton 句柄，发生错误时为
/// `NULL`
///
/// @return 函数成功返回 `true`, 发生错误时返回 `false`
bool rctxCreateDynamicBuffer(
    RendererContext*          pContext,
    VkBufferUsageFlagBits     usage,
    size_t                    bufferSize,
    uint64_t                  dataOffset,
    size_t                    dataSize,
    const void*               pData,
    VkBuffer*                 outBuffer,
    VmaAllocation*            outAllocation
);


/// @brief 对指定的动态缓冲区进行数据更新.
///
/// @warning ThreadUnSafe: 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会调
/// 用 rctxEndFrame() 改变渲染器的当前飞行帧状态，那么该函数就是线程安全的
///（注：更新目标缓冲区可不是）
///
/// @param bufferSize 调用该函数需要传入指定动态缓冲区（用户创建时指定的）的总大小 
/// @param dataOffset 更新数据时应用的偏移量
/// @param dataSize 更新数据的大小
/// @param pData 要更新的数据
/// @param allocation 指定动态缓冲区对应的 VmaAllocation 句柄
void rctxUpdateDynamicBuffer(
    RendererContext*          pContext,
    size_t                    bufferSize,
    uint32_t                  dataOffset,
    size_t                    dataSize,
    const void*               pData,
    VmaAllocation             allocation
);


/// @brief 请求销毁 Buffer 资源，该函数不会立即执行销毁，而是会将传入的资源暂存入当前飞行帧下的
/// 销毁列表中待销毁，其会在你下次调用对应的 rctxBeginFrame() 时刷新以销毁所有在内的资源来保证
/// 飞行帧之间的数据同步安全.
///
/// 在调用该函数后，你不应该以任何方式再次使用已被请求销毁的 Buffer 资源.
///
/// 你必须在调用 rctxBeginFrame() 之后，调用 rctxEndFrame() 之前调用该函数以表达正确的具体飞行
/// 帧状态下提交销毁的意图. 
///
/// @warning ThreadUnSafe: 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会
/// 调用 rctxEndFrame() 改变渲染器的当前飞行帧状态，那么该函数就是线程安全的
///
/// @return 当请求成功时返回 `true`，请求失败返回 `false` —— 这时你需要自行调用
/// rctxDangerousDestroyBuffer() 才能销毁对应资源
bool rctxRequestDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
);


/// @brief 等待渲染器上下文的设备处于空闲状态.
void rctxWaitIdle(RendererContext* pContext);


/// @brief 对所有的销毁列表（包括飞行帧副本）中当前记录的资源进行销毁并移除.
///
/// @warning 你必须在调用了 rctxWaitIdle() 确保任何资源均不被占用后才应该调用该函数，否则极可能
/// 会导致对处于被占用状态的资源进行销毁，从而引发程序崩溃.
///
/// @param pContext 渲染器上下文句柄
void rctxDeletionListDangerousFlushALL(RendererContext* pContext);


/// @brief 立即销毁给定的 Buffer 资源.
///
/// 只有你在确保该 Buffer 资源不处于被占用状态时（例如程序退出并调用 rctxWaitIdle() 后）才应调
/// 用该函数.
void rctxDangerousDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
);


/// @brief 分配一个 Camera 描述符集对象，并将给定的资源对象更新写入对应 Binding.
///
/// @param pCameraUniformBufferInfo 写入 CameraUniformBuffer Binding 要用到的缓冲区资源描述
/// 信息
/// @param outDescriptorSet 输出参数，返回分配好的 Camera 描述符集对象
///
/// @return 分配成功后返回 `true`，发生错误时返回 `false`
bool rctxAllocateCameraDescriptorSet(
    RendererContext*                pContext,
    const VkDescriptorBufferInfo*   pCameraUniformBufferInfo,
    VkDescriptorSet*                outDescriptorSet
);


/// @brief 分配一个 DrawItem 描述符集对象，并将给定的资源对象更新写入对应 Binding.
///
/// @param pDrawItemUniformBufferInfo 写入 DrawItemUniformBuffer Binding 要用到的缓冲区资源
/// 描述信息
/// @param outDescriptorSet 输出参数，返回分配好的 DrawItem 描述符集对象
///
/// @return 分配成功后返回 `true`，发生错误时返回 `false`
bool rctxAllocateDrawItemDescriptorSet(
    RendererContext*                pContext,
    const VkDescriptorBufferInfo*   pDrawItemUniformBufferInfo,
    VkDescriptorSet*                outDescriptorSet
);


/// @brief 调用该函数会等待当前飞行帧栅栏，以保证后续当前飞行帧下的相关数据同步安全.
///
/// 该函数返回便代表着当前飞行帧下的资源已不再处于占用状态，这时应用端可以安全地在当前飞行帧下进行
/// 相关数据操作如 资源更新 等直至调用 rctxDrawFrame().
///
/// @warning 你需要在调用 资源更新 等相关函数之前调用该函数以保证当前飞行帧下的资源同步安全！
///
/// 你需要在调用 rctxDrawFrame() 之前调用该函数以保证当前飞行帧下的数据同步安全！
///
/// @param pContext 渲染器上下文句柄
void rctxBeginFrame(RendererContext* pContext);


/// @brief DrawFrame 函数，根据你传入的通道管线绘制信息，在当前飞行帧状态下进行绘制.
///
/// 函数内部包括了请求交换链图像、检查是否重建交换链、录制主命令缓冲区、提交渲染与呈现一系列操作.
///
/// @warning 调用该函数前你需要先调用 rctxBeginFrame() 进行等待以保证当前飞行帧下的数据同步安全！
///
/// 在调用该函数后，当前飞行帧下的资源将会几乎立刻处于被占用状态，如无其他事务，你应该立即调用
/// rctxEndFrame() 以将渲染器状态切换到下一飞行帧，否则在那之前渲染器会一直处于当前飞行帧状态下，
/// 你此时进行的任何资源修改，都极可能因为占用而造成数据竞争.
///
///（除非你再次调用 rctxBeginFrame() 以等待当前飞行帧栅栏，让数据不再处于占用状态. 但这样等待会
/// 造成大量性能浪费，所以推荐当前飞行帧的工作提交完后（调用了 rctxDrawFrame()）便可快速切换渲染
/// 器状态至下一飞行帧中继续工作，以发挥最佳的渲染并行性）
///
/// @param pContext 渲染器上下文句柄
/// @param isWindowFramebufferResized 平台窗口的帧缓冲区尺寸是否发生变更，该参数用于函数内部
/// 判断是否需重建交换链
/// @param windowFramebufferWidth 平台窗口帧缓冲区宽，用于作重建交换链的参数
/// @param windowFramebufferHeight 平台窗口帧缓冲区高，用于作重建交换链的参数
/// @param pMainRenderPassPipelinesDrawInfo 主渲染通道所有管线的对应绘制信息
void rctxDrawFrame(
    RendererContext*        pContext,
    bool                    isWindowFramebufferResized,
    int                     windowFramebufferWidth,
    int                     windowFramebufferHeight,
    MainRenderPassDrawInfo* pMainRenderPassDrawInfo
);


/// @brief 调用该函数将渲染器上下文状态切换至下一飞行帧.
///
/// @warning 在调用 vrdrDrawFrame() 后如无其他事务，你应该调用该函数，为下次 vrdrDrawFrame()
/// 的调用做准备.
///
/// @param pContext 渲染器上下文句柄
void rctxEndFrame(RendererContext* pContext);