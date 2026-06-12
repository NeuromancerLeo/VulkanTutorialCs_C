#pragma once

#include <stdbool.h>

#include <vulkan/vulkan.h>
#include "vulkan/vma/vk_mem_alloc.h"

#include "../common/nativelib.h"

typedef void* Window;
typedef VkSurfaceKHR (*SurfaceCreateHelperFunc)(VkInstance);
typedef struct MainRenderPassDrawInfo MainRenderPassDrawInfo;


/// @brief 初始化 vkrenderer 库.
///
/// @param windowExtensionCount 初始化 vkrenderer 库需要提供外部平台窗口所需扩展数
/// @param windowExtensionStrings 外部平台窗口所有所需扩展的对应字符串
/// @param create_window_surface_helper 外部平台窗口针对 VkSurface 创建的辅助函数指针
/// @param windowFramebufferWidth 外部平台窗口帧缓冲区宽度
/// @param windowFramebufferHeight 外部平台窗口帧缓冲区高度
///
/// @return 成功返回 `true`，发生错误返回 `false`
EX_API bool vrdrInitialize(
    uint32_t                windowExtensionCount,
    const char**            windowExtensionStrings,
    SurfaceCreateHelperFunc create_window_surface_helper,
    int                     windowFramebufferWidth,
    int                     windowFramebufferHeight
);


/// @brief 该函数用于供你注册成外部平台窗口帧缓冲区尺寸发生变化时的回调.
///
/// @warning vkrenderer 需要这个函数在平台窗口帧缓冲区尺寸发生变化时被作为回调调用，这样才能保证
/// 内部的即时响应，以正确触发交换链重建.
EX_API void vkrdrFramebufferResizeCallback(Window* window, int width, int height);


/// @brief 创建一个顶点用的静态缓冲区.
///
/// @note ThreadSafe: 该函数不依赖任何运行时状态，已被设计成线程安全的，你可以任意使用不同的线
/// 程来调用该函数.
///
/// @param dataSize 顶点数据大小
/// @param pVertexData 指向顶点数据的指针
/// @param outBuffer 输出变量，返回 VkBuffer 句柄，发生错误时为 `NULL`
/// @param outAllocation 输出变量，返回 VkBuffer 对应的 VmaAllocaiton 句柄，发生错误时为
/// `NULL`
///
/// @return 函数成功返回 `true`, 发生错误时返回 `false`
EX_API bool vrdrCreateStaticVertexBuffer(
    size_t              dataSize,
    const void*         pVertexData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


/// @brief 创建一个顶点索引用的静态缓冲区.
///
/// @note ThreadSafe: 该函数不依赖任何运行时状态，已被设计成线程安全的，你可以任意使用不同的线
/// 程来调用该函数.
///
/// @param dataSize 顶点索引数据大小
/// @param pVertexData 指向顶点索引数据的指针
/// @param outBuffer 输出变量，返回 VkBuffer 句柄，发生错误时为 `NULL`
/// @param outAllocation 输出变量，返回 VkBuffer 对应的 VmaAllocaiton 句柄，发生错误时为
/// `NULL`
///
/// @return 函数成功返回 `true`, 发生错误时返回 `false`
EX_API bool vrdrCreateStaticIndexBuffer(
    uint32_t            dataSize,
    const uint32_t*     pIndexData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


/// @brief 获取渲染器对 Uniform 动态缓冲区所规定的最小偏移量对齐要求值，当你打算构建一个用作
/// Uniform 动态缓冲区的单元数据结构时，你必须负责保证其大小以及创建 Uniform 动态缓冲区时设置
/// 的总大小满足该函数返回值的整倍数，否则渲染器管线将无法通过你在绘制信息中传入的动态偏移量
///（Dynamic Offset）来正确读取和使用目标缓冲区内的 Uniform 变量.
///
/// @return 渲染器对 Uniform 动态缓冲区所规定的最小偏移对齐要求值
EX_API uint32_t vrdrGetMinimalUniformBufferOffsetAlignment();


/// @brief 创建一个 Uniform 变量用的动态缓冲区（该类缓冲区支持动态偏移量，也就是说可以作为大
/// Uniform Buffer 使用）.
///
/// @warning 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会调用
/// vrdrEndFrame() 改变渲染器的当前飞行帧状态，那么该函数就是线程安全的.
///
/// @param bufferSize 用户期望的缓冲区大小，其必须是
/// vrdrGetMinimalUniformBufferOffsetAlignment() 返回的值的整数倍
/// @param dataOffset 上传 Uniform 数据时应用的偏移量，其必须是
/// vrdrGetMinimalUniformBufferOffsetAlignment() 返回的值的整数倍
/// @param dataSize 上传 Uniform 数据的大小
/// @param pUniformData 指向 Uniform 数据的指针
/// @param outBuffer 输出变量，返回 VkBuffer 句柄，发生错误时为 `NULL`
/// @param outAllocation 输出变量，返回 VkBuffer 对应的 VmaAllocaiton 句柄，发生错误时为
/// `NULL`
///
/// @return 函数成功返回 `true`, 发生错误时返回 `false`
EX_API bool vrdrCreateDynamicUniformBuffer(
    size_t              bufferSize,
    uint32_t            dataOffset,
    size_t              dataSize,
    const void*         pUniformData,
    VkBuffer*           outBuffer,
    VmaAllocation*      outAllocation
);


/// @brief 对指定的动态缓冲区进行数据更新.
///
/// @warning 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会调用
/// rctxEndFrame() 改变渲染器的当前飞行帧状态，那么该函数就是线程安全的
///（注：更新目标缓冲区可不是）
///
/// @param bufferSize 调用该函数需要传入指定动态缓冲区（用户创建时指定的）的总大小
/// @param dataOffset 更新数据时应用的偏移量
/// @param dataSize 更新数据的大小
/// @param pData 指向更新数据的指针
/// @param allocation 指定动态缓冲区对应的 VmaAllocation 句柄
EX_API void vrdrUpdateUniformBuffer(
    size_t          bufferSize,
    uint32_t        dataOffset,
    size_t          dataSize,
    void*           pData,
    VmaAllocation   allocation
);


/// @brief 请求销毁给定的缓冲区资源，该函数不会立即执行销毁，而是会将传入的资源句柄暂存入渲染器
/// 当前飞行帧下的销毁列表中. 销毁列表会在你下次调用 vrdrBeginFrame() 时刷新以销毁释放所有记录
/// 在内的资源来保证不同飞行帧之间的数据同步安全.
///
/// @warning 在调用该函数后，你不应该以任何方式再次使用已被你请求销毁的 Buffer 资源.
///
/// 你必须在调用 vrdrBeginFrame() 之后，调用 vrdrEndFrame() 之前调用该函数以表达正确的具体飞
/// 行帧状态下提交销毁的意图. 
///
/// ThreadUnSafe: 该函数依赖当前飞行帧状态，所以只要用户保证在调用该函数期间，绝对不会调用
/// rctxEndFrame() 改变渲染器的当前飞行帧状态，那么该函数就是线程安全的.
///
/// @return 当请求成功时返回 `true`，请求失败返回 `false` —— 这时你需要自行调用
/// vrdrDangerousDestroyBuffer() 才能够销毁对应资源
EX_API bool vrdrRequestDestroyBuffer(VkBuffer buffer, VmaAllocation allocation);


/// @brief 阻塞等待渲染器进入空闲状态. 
///
/// 该函数一般用于在程序销毁阶段或性能不敏感时机进行阻塞以保证资源能够安全销毁.
EX_API void vrdrWaitIdle();


/// @brief 对渲染器中所有的销毁列表（包括飞行帧副本）中当前记录的资源进行销毁并移除.
///
/// @warning 你必须在调用了 vrdrWaitIdle() 确保任何资源均不被占用后才应该调用该函数，否则极可能
/// 会导致对处于被占用状态的资源进行销毁，从而引发程序崩溃.
EX_API void vrdrDeletionListDangerousFlushALL();


/// @brief 立即销毁给定的缓冲区资源.
///
/// 只有你在确保该缓冲区资源不处于被占用状态时（例如程序退出并调用 vrdrWaitIdle() 后）才应调用
/// 该函数.
EX_API void vrdrDangerousDestroyBuffer(VkBuffer buffer, VmaAllocation allocation);


/// @brief 分配一个 Camera 描述符集对象，并将给定的资源对象更新写入对应 Binding.
///
/// @param cameraUniformBufferOffset 将 Buffer 资源更新至 CameraUniformBuffer Binding 时要
/// 应用的偏移量
/// @param cameraUniformBufferRange 将 Buffer 资源更新至 CameraUniformBuffer Binding 时要
/// 应用的范围量（偏移量 + 范围量为最终 的资源更新应用范围）
/// @param cameraUniformBuffer 要更新至描述符集 CameraUniformBuffer Binding 的 Buffer 资源
/// @param outDescriptorSet 输出参数，返回分配好的 Camera 描述符集对象
///
/// @return 分配成功后返回 `true`，发生错误时返回 `false`
EX_API bool vrdrAllocateCameraDescriptorSet(
    size_t              cameraUniformBufferOffset,
    size_t              cameraUniformBufferRange,
    const VkBuffer      cameraUniformBuffer,
    VkDescriptorSet*    outDescriptorSet
);


/// @brief 分配一个 DrawItem 描述符集对象，并将给定的资源对象更新写入对应 Binding.
///
/// @param cameraUniformBufferOffset 将 Buffer 资源更新至 DrawItemUniformBuffer Binding 时
/// 要应用的偏移量
/// @param cameraUniformBufferRange 将 Buffer 资源更新至 DrawItemUniformBuffer Binding 时要
/// 应用的范围量（偏移量 + 范围量为最终 的资源更新应用范围）
/// @param cameraUniformBuffer 要更新至描述符集 DrawItemUniformBuffer Binding 的 Buffer 资源
/// @param outDescriptorSet 输出参数，返回分配好的 DrawItem 描述符集对象
///
/// @return 分配成功后返回 `true`，发生错误时返回 `false`
EX_API bool vrdrAllocateDrawItemDescriptorSet(
    size_t              drawItemUniformBufferOffset,
    size_t              drawItemUniformBufferRange,
    const VkBuffer      drawItemUniformBuffer,
    VkDescriptorSet*    outDescriptorSet
);


/// @brief 调用该函数会等待渲染器当前飞行帧栅栏，以保证后续当前飞行帧下的相关数据同步安全.
///
/// 该函数返回便代表着当前飞行帧下的资源已不再处于占用状态，这时你可以安全地在当前飞行帧下进行相关
/// 数据操作如 资源更新 等直至调用 vrdrDrawFrame().
///
/// @warning 你需要在调用 资源更新 等相关函数之前调用该函数以保证当前飞行帧下的资源同步安全！
///
/// 你需要在调用 vrdrDrawFrame() 之前调用该函数以保证当前飞行帧下的数据同步安全！
EX_API void vrdrBeginFrame();


/// @brief DrawFrame 函数，渲染器根据你传入的通道管线绘制信息，在当前飞行帧状态下进行命令录制并
/// 提交渲染与呈现，所有工作一经提交后该函数便立刻返回.
///
/// @warning 调用该函数前你需要先调用 vrdrBeginFrame() 进行等待以保证当前飞行帧下的数据同步安全！
///
/// 在调用该函数后，当前飞行帧下的资源将会几乎立刻处于被占用状态，如无其他事务，你应该立即调用
/// vrdrEndFrame() 以将渲染器状态切换到下一飞行帧，否则在那之前渲染器会一直处于当前飞行帧状态下，
/// 你此时进行的任何资源修改，都极可能因为占用而造成数据竞争.
///
///（除非你再次调用 vrdrBeginFrame() 以等待当前飞行帧栅栏，让数据不再处于占用状态. 但这样等待会
/// 造成大量性能浪费，所以推荐当前飞行帧的工作提交完后（调用了 vrdrDrawFrame()）便可快速切换渲染
/// 器状态至下一飞行帧中继续工作，以发挥最佳的渲染并行性）
///
/// @param isWindowFramebufferResized 平台窗口的帧缓冲区尺寸是否发生变更，该参数用于函数内部
/// 判断是否需重建交换链
/// @param windowFramebufferWidth 平台窗口帧缓冲区宽，用于作重建交换链的参数
/// @param windowFramebufferHeight 平台窗口帧缓冲区高，用于作重建交换链的参数
/// @param pMainRenderPassPipelinesDrawInfo 渲染器主渲染通道所有管线的对应绘制信息
EX_API void vrdrDrawFrame(
    MainRenderPassDrawInfo  *pMainRenderPassDrawInfo,
    int                     windowFramebufferWidth,
    int                     windowFramebufferHeight
);


/// @brief 调用该函数将渲染器状态切换至下一飞行帧.
///
/// @warning 在调用 vrdrDrawFrame() 后如无其他事务，你应该调用该函数，为下次 vrdrDrawFrame()
/// 的调用做准备.
EX_API void vrdrEndFrame();


EX_API void vrdrTerminate();