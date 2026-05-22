#pragma once

#include <stdbool.h>

#include <vulkan/vulkan.h>
#include "../vulkan/vma/vk_mem_alloc.h"
#include <GLFW/glfw3.h>

typedef struct RendererContext RendererContext;
typedef struct MainRenderPassDrawInfo MainRenderPassDrawInfo;


/// @brief 为一个渲染器上下文分配内存并返回其句柄.
///
/// @return 一个新的 RendererContext 的句柄，发生错误时返回 `NULL`
RendererContext* rctxNewRendererContext();


/// @brief 给定一个渲染器上下文然后对其进行（初步地）初始化构建.
///
/// @param context 目标渲染器上下文句柄
/// @param window 构建渲染器上下文需要对应的窗口句柄
///
/// @return 当构建成功时返回 `true`，若发生错误则会终止构建（相关函数会输出信息）并返回 `false`
bool rctxCreateRendererContext(RendererContext* pContext, GLFWwindow* window);


/// @brief 给定渲染器上下文句柄，销毁其（除了窗口句柄外的）所有上下文对象，同时销毁自身释放内存
///
/// @param pContext 要销毁的渲染器上下文句柄
void rctxDestroyRendererContext(RendererContext* pContext);


/// @brief 创建并填充静态性缓冲区，内部使用 StagingBuffer 并使用传送队列复制数据到设备本地的
/// Buffer 上.
///
/// 该函数不依赖任何运行时状态，已被设计成线程安全的，你可以任意使用不同的线程来调用该函数.
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
/// 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会调用 rctxEndFrame() 改
/// 变渲染器的当前飞行帧状态，那么该函数就是线程安全的.
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
/// 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会调用 rctxEndFrame() 改
/// 变渲染器的当前飞行帧状态，那么该函数就是线程安全的.
///
/// @param bufferSize 调用该函数需要传入指定动态缓冲区的总大小 
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
/// 删除列表中待销毁，其会在你下次调用对应的 rctxBeginFrame() 时刷新以销毁所有在内的资源来保证
/// 飞行帧之间的数据同步安全.
///
/// 在调用该函数后，你不应该以任何方式再次使用已被请求销毁的 Buffer 资源.
///
/// 你必须在调用 rctxBeginFrame() 之后，调用 rctxEndFrame() 之前调用该函数以表达正确的飞行帧
/// 状态下提交销毁的意图. 
///
/// @return 当请求成功时返回 true，请求失败返回 false —— 这时你需要自行调用
/// rctxDestroyBuffer() 才能销毁对应资源
bool rctxRequestDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
);


/// @brief 等待渲染器上下文的设备处于空闲状态.
void rctxWaitIdle(RendererContext* pContext);


/// @brief 立即销毁 Buffer 资源.
///
/// 只有你在确保 Buffer 资源不会被占用时（如程序退出并调用 rctxWaitIdle() 后）才应调用该函数.
void rctxDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
);


/// @brief 创建一个相机描述符集对象，并为其更新给定的资源对象.
///
/// @param bufferOffset 将 uniformBuffer 更新至描述符集时要应用的偏移量
/// @param bufferRange 将 uniformBuffer 更新至描述符集时要应用的范围量（偏移量 + 范围量为最终
/// 的资源更新应用范围）
/// @param uniformBuffer 要更新至描述符集的 Buffer 资源
/// @param outDescriptorSet 输出参数，返回创建的相机描述符集对象
///
/// @return 函数成功后返回 `true`，发生错误时返回 `false`
bool rctxCreateCameraDescriptorSet(
    RendererContext*                pContext,
    size_t                          bufferOffset,
    size_t                          bufferRange,
    VkBuffer                        uniformBuffer,
    VkDescriptorSet*                outDescriptorSet
);


/// @brief 创建一个 DrawItems 描述符集对象，并为其更新给定的资源对象.
///
/// @param bufferOffset 将 uniformBuffer 更新至描述符集时要应用的偏移量
/// @param bufferRange 将 uniformBuffer 更新至描述符集时要应用的范围量（偏移量 + 范围量为最终
/// 的资源更新应用范围）
/// @param uniformBuffer 要更新至描述符集的 Buffer 资源
/// @param outDescriptorSet 输出参数，返回创建的 DrawItems 描述符集对象
///
/// @return 函数成功后返回 `true`，发生错误时返回 `false`
bool rctxCreateDrawItemsDescriptorSet(
    RendererContext*                pContext,
    size_t                          bufferOffset,
    size_t                          bufferRange,
    VkBuffer                        uniformBuffer,
    VkDescriptorSet*                outDescriptorSet
);


/// @brief 调用该函数会等待当前飞行帧栅栏，以保证当前飞行帧的相关数据同步安全.
///
/// 该函数返回后便代表着当前飞行帧下的资源已不再处于占用状态，这时应用端可以安全地进行相关数据
/// 操作如 资源更新 等直至调用 rctxDrawFrame().
///
/// 你需要在调用 资源更新 等相关函数之前调用该函数以保证当前飞行帧下的资源同步安全！
///
/// 你需要在调用 rctxDrawFrame() 之前调用该函数以保证当前飞行帧下的数据同步安全！
///
/// @param pContext 渲染器上下文句柄
void rctxBeginFrame(RendererContext* pContext);


/// @brief DrawFrame 函数，根据你传入的通道管线绘制信息针对当前飞行帧进行绘制.
///
/// 函数内部包括了请求交换链图像、检查是否重建交换链、录制主命令缓冲区、提交渲染与呈现一系列操作.
///
/// 你需要在调用 rctxBeginFrame() 之后再调用该函数以保证当前飞行帧下的数据同步安全！
///
/// 在调用该函数后，当前飞行帧下的资源将会几乎立刻处于被占用状态，如无其他事务，你应该立即调用
/// rctxEndFrame() 以将渲染器状态切换到下一飞行帧，否则在那之前你会一直处于当前飞行帧状态下，
/// 你进行的任何资源修改，都极可能因为占用而造成数据竞争.
///
///（除非你再次调用 rctxBeginFrame() 以等待当前飞行帧栅栏，让数据不再处于占用状态. 但这样等待会
/// 造成大量性能浪费，所以推荐当前飞行帧的工作提交完后（调用了 rctxDrawFrame()）便可快速切换渲染
/// 器状态至下一飞行帧中继续工作，以发挥最佳的渲染并行性）
///
/// @param pContext 渲染器上下文句柄
/// @param isFramebufferResized 窗口的帧缓冲区尺寸是否发生变更，该参数用于函数内部判断是否需重建
/// 交换链
/// @param pMainRenderPassPipelinesDrawInfo 主渲染通道所有管线的对应绘制信息
void rctxDrawFrame(
    RendererContext*                    pContext,
    bool                                isFramebufferResized,
    // MainRenderPassPipelinesDrawInfo*    pMainRenderPassPipelinesDrawInfo
    MainRenderPassDrawInfo*             pMainRenderPassDrawInfo
);


/// @brief 调用该函数将渲染器上下文状态切换至下一飞行帧.
///
/// 你应该在调用 rctxDrawFrame() 后调用该函数，为下次 rctxDrawFrame() 的调用做准备.
///
/// @param pContext 渲染器上下文句柄
void rctxEndFrame(RendererContext* pContext);