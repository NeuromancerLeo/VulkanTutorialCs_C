#pragma once

#include <stdbool.h>

#include <vulkan/vulkan.h>
#include "../vma/vk_mem_alloc.h"


/// @brief 创建 VkInstance，其是程序和 Vulkan 库之间的接口.
///
/// @param windowExtensionCount 平台窗口所需扩展数
/// @param windowExtensionStrings 平台窗口所需所有扩展的对应字符串
/// 
/// @return 返回新创建的 VkInstance 句柄（当发生错误时返回 `NULL`）
VkInstance vwrpCreateInstance(
    uint32_t        windowExtensionCount,
    const char**    windowExtensionStrings
);


/// @brief 销毁给定的 VkInstance.
void vwrpDestroyInstance(VkInstance instance);


/// @brief 销毁给定的 VkSurfaceKHR.
///
/// @param instance 调用该函数需要传入一个对应的 VkInstance 句柄
void vwrpDestroySurface(VkInstance instance, VkSurfaceKHR surface);


/// @brief 查询可用物理设备并尝试选择可用的显卡作 PhysicalDevice.
///
/// @param instance 调用该函数需要传入一个有效的 VkInstance 句柄
/// @param surface 调用该函数需要传入一个有效的 VkSurfaceKHR 句柄
///
/// @return 返回一个可用的 PhysicalDevice 句柄（当发生错误时返回 `NULL`）
VkPhysicalDevice vwrpPickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);


/// @brief 根据给定物理设备创建逻辑设备.
///
/// @param pGraphicsQueueFamilyIndex 输出参数，输出对应图形队列的所属队列族索引，错误时为 -1
/// @param pGraphicsQueue 输出参数，输出有效的 VkQueue 句柄（graphics），发生错误时为 `NULL`
/// @param pPresentationQueueFamilyIndex 输出参数，输出对应呈现队列的所属队列族索引，错误时为 -1
/// @param pPresentationQueue 输出参数，输出有效的 VkQueue 句柄（presentation），
/// @param pTransferQueueFamilyIndex 输出参数，输出对应传输队列的所属队列族索引，错误时为 -1
/// @param pTransferQueue 输出参数，输出有效的 VkQueue 句柄（transfer），发生错误时为 `NULL`
/// 发生错误时为 `NULL`
///
/// @return 返回新创建的 VkDevice 句柄（当发生错误时返回 `NULL`）
VkDevice vwrpCreateLogicalDevice(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    uint32_t*           pGraphicsQueueFamilyIndex,
    VkQueue*            pGraphicsQueue,
    uint32_t*           pPresentationQueueFamilyIndex,
    VkQueue*            pPresentationQueue,
    uint32_t*           pTransferQueueFamilyIndex,
    VkQueue*            pTransferQueue
);


/// @brief 销毁给定的 VkDevice.
void vwrpDestroyLogicalDevice(VkDevice device);


/// @brief 创建 VMA 分配器.
///
/// @param instance 调用该函数需要传入一个有效的 VkInstance 句柄
/// @param physicalDevice 调用该函数需要传入一个有效的 VkPhysicalDevice 句柄
/// @param device 调用该函数需要传入一个有效的 VkDevice 句柄
///
/// @return 返回新创建的 VmaAllocator 句柄（当发生错误时返回 `NULL`）
VmaAllocator vwrpCreateVmaAllocator(
    VkInstance          instance,
    VkPhysicalDevice    physicalDevice,
    VkDevice            device
);


/// @brief 销毁给定的 VmaAllocator（该函数调用必须在 VkDevice 被销毁之前）. 
void vwrpDestroyVmaAllocator(VmaAllocator allocator);


/// @brief 为给定 Surface 创建交换链.
///
/// @param windowFramebufferWidth 对应平台窗口缓冲区的像素宽度
/// @param windowFramebufferHeight 对应平台窗口缓冲区的像素高度
/// @param surface 给定 Surface 句柄
/// @param physicalDevice 给定物理设备句柄
/// @param device 给定设备句柄
/// @param oldSwapchain 旧的交换链句柄
/// @param pSwapchainImageCount 输出参数，交换链创建后其输出交换链图像句柄数组的大小
/// @param ppSwapchainImages 输出参数，其输出一个指向交换链图像句柄数组的指针
/// @param pSwapchainImageFormat 输出参数，其输出交换链图像的格式
/// @param pSwapchainExtent 输出参数，其输出交换链图像的范围
///
/// @return 返回新创建的 VkSwapchainKHR 句柄（当发生错误时返回 `NULL`）
VkSwapchainKHR vwrpCreateSwapchain(
    int                 windowFramebufferWidth,
    int                 windowFramebufferHeight,
    VkSurfaceKHR        surface,
    VkPhysicalDevice    physicalDevice, 
    VkDevice            device,
    VkSwapchainKHR      oldSwapchain,
    uint32_t*           pSwapchainImageCount,
    VkImage**           ppSwapchainImages,
    VkFormat*           pSwapchainImageFormat,
    VkExtent2D*         pSwapchainExtent    
);


/// @brief 销毁给定的 VkSwapchainKHR.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param swapchain 要销毁的交换链句柄
/// @param ppSwapchainImages 需同时传入在 vwrpCreateSwapchain() 中分配的交换链图像数组供销毁
/// (可传入 `NULL`，这时函数不会帮你销毁交换链图像数组，需要你自己销毁，例如你在重建交换链，
/// 需分开销毁交换链和其图像数组)
void vwrpDestroySwapchain(
    VkDevice        device, 
    VkSwapchainKHR  swapchain, 
    VkImage**       ppSwapchainImages
);


/// @brief 成功创建交换链后调用该函数为交换链中的每一个图像创建基本的图像视图（VkImageView）.
///
/// @param device 调用该函数需要传入对应的 VkDevice 句柄
/// @param swapchainImageFormat 指定要创建的图像视图的图像格式
/// @param swapchainImageCount 指定要创建的图像视图的数量
/// @param pSwapchainImages 调用该函数需要传入对应的交换链图像数组
///
/// @return 创建成功后返回一个属于交换链的图像视图数组，失败则返回 `NULL`
VkImageView* vwrpCreateSwapchainImageViews(
    VkDevice        device,
    VkFormat        swapchainImageFormat,
    uint32_t        swapchainImageCount,
    const VkImage*  pSwapchainImages
);


/// @brief 销毁交换链所有的图像视图.
///
/// @param device 调用该函数需要传入对应的 VkDevice 句柄
/// @param swapchainImageCount 交换链图像总数
/// @param ppSwapchainImageViews 要销毁的交换链图像视图的数组的地址
///（数组本身也会被销毁以释放内存）
void vwrpDestroySwapchainImageViews(
    VkDevice        device,
    uint32_t        swapchainImageCount,
    VkImageView**   ppSwapchainImageViews
);


/// @brief 创建命令池，其用于分配命令缓冲区.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的命令池创建信息
///
/// @return 返回新创建的 VkCommandPool 句柄（当发生错误时返回 `NULL`）
VkCommandPool vwrpCreateCommandPool(
    VkDevice                        device,
    const VkCommandPoolCreateInfo*  pCreateInfo
);


/// @brief 销毁给定的 VkCommandPool.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param commandPool 要销毁的命令池句柄
void vwrpDestroyCommandPool(VkDevice device, VkCommandPool commandPool);


/// @brief 分配（多个）命令缓冲区.
///
/// 注：该函数是外部同步的，因为使用了线程不安全的 `VkCommandPool` 对象
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pAllocateInfo 给定的命令缓冲区分配信息
/// @param pCommandBuffers 输出参数，函数失败时输出 `NULL`
void vwrpAllocateCommandBuffers(
    VkDevice                            device,
    const VkCommandBufferAllocateInfo*  pAllocateInfo,
    VkCommandBuffer*                    pCommandBuffers
);


/// @brief 开始录制给定命令缓冲区.
///
/// 注：该函数是外部同步的，因为使用了线程不安全的 `VkCommandBuffer` 对象
///
/// @param label 描述性标签，仅供输出日志信息用
/// @param commandBuffer 目标命令缓冲区
/// @param pBeginInfo 给定的开始录制信息
///
/// @return 函数执行成功返回 `true`，反之遇到错误返回 `false`
bool vwrpBeginCommandBuffer(
    const char*                     label,
    VkCommandBuffer                 commandBuffer,
    const VkCommandBufferBeginInfo  *pBeginInfo
);


/// @brief 结束录制给定命令缓冲区.
///
/// 注：该函数是外部同步的，因为使用了线程不安全的 `VkCommandBuffer` 对象
///
/// @param label 描述性标签，仅供输出日志信息用
/// @param commandBuffer 目标命令缓冲区
///
/// @return 函数执行成功返回 `true`，反之遇到错误返回 `false`
bool vwrpEndCommandBuffer(const char* label, VkCommandBuffer commandBuffer);


/// @brief 创建渲染通道，其用于为管线提供目标帧缓冲区中关于附件的信息.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的渲染通道创建信息
///
/// @return 返回新创建的 VkRenderPass 句柄（当发生错误时返回 `NULL`）
VkRenderPass vwrpCreateRenderPass(
    VkDevice                        device,
    const VkRenderPassCreateInfo*   pCreateInfo
);


/// @brief 销毁给定的 VkRenderPass.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param renderPass 要销毁的渲染通道句柄
void vwrpDestroyRenderPass(VkDevice device, VkRenderPass renderPass);


/// @brief 创建帧缓冲区，其用作管线渲染的目标，帧缓冲区包含了具体的附件
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的帧缓冲区创建信息
///
/// @return 返回新创建的 VkFramebuffer 句柄（当发生错误时返回 `NULL`）
VkFramebuffer vwrpCreateFramebuffer(
    VkDevice                          device,
    const VkFramebufferCreateInfo*    pCreateInfo
);


/// @brief 销毁给定的 VkFramebuffer.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param framebuffer 要销毁的帧缓冲区句柄
void vwrpDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer);


/// @brief 创建着色器模块，其用于为管线提供着色器阶段的着色器代码源.
///
/// @param device 调用该函数需要传入对应的 VkDevice 句柄
/// @param spvFilePath 对应 SPIR-V 文件的目录
///
/// @return 返回新创建的 VkShaderModule 句柄（当发生错误时返回 `NULL`）
VkShaderModule vwrpCreateShaderModule(
    VkDevice        device,
    const char*     spvFilePath
);


/// @brief 销毁给定的 VkShaderModule.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param shaderModule 要销毁的着色器模块句柄
void vwrpDestroyShaderModule(VkDevice device, VkShaderModule shaderModule);


/// @brief 创建描述符集布局，其用于定义一个描述符集的布局. 
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的描述符集布局创建信息
///
/// @return 返回新创建的 VkDescriptorSetLayout 句柄（当发生错误时返回 `NULL`）
VkDescriptorSetLayout vwrpCreateDescriptorSetLayout(
    VkDevice                                device,
    const VkDescriptorSetLayoutCreateInfo*  pCreateInfo
);


/// @brief 销毁给定的 VkDescriptorSetLayout. 
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param descriptorSetLayout 要销毁的描述符集布局句柄
void vwrpDestroyDescriptorSetLayout(
    VkDevice                device,
    VkDescriptorSetLayout   descriptorSetLayout
);


/// @brief 创建管线布局，其用于为管线提供描述符集的信息.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的管线布局创建信息
///
/// @return 返回新创建的 VkPipelineLayout 句柄（当发生错误时返回 `NULL`）
VkPipelineLayout vwrpCreatePipelineLayout(
    VkDevice                            device,
    const VkPipelineLayoutCreateInfo*   pCreateInfo
);


/// @brief 销毁给定的 VkPipelineLayout.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pipelineLayout 要销毁的管线布局句柄
void vwrpDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout);


/// @brief 创建图形管线.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的管线创建信息
///
/// @return 返回新创建的 VkPipeline 句柄（当发生错误时返回 `NULL`）
VkPipeline vwrpCreateGraphicsPipeline(
    VkDevice                             device,
    const VkGraphicsPipelineCreateInfo*  pCreateInfo
);


/// @brief 销毁给定的 VkPipeline.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pipeline 要销毁的管线句柄
void vwrpDestroyPipeline(VkDevice device, VkPipeline pipeline);


VkDescriptorPool vwrpCreateDescriptorPool(
    VkDevice                            device,
    const VkDescriptorPoolCreateInfo*   pCreateInfo 
);


void vwrpDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool);


/// @brief 创建一个信号量，用于在 GPU 端等待 GPU 端完成提交的命令.
VkSemaphore vwrpCreateSemaphore(
    const char*             label,
    VkDevice                device,
    VkSemaphoreCreateInfo*  pCreateInfo    
);


/// @brief 销毁给定的 VkSemaphore. 
void vwrpDestroySemaphore(VkDevice device, VkSemaphore semaphore);


/// @brief 创建一个栅栏，用于在 CPU 端等待 GPU 端完成提交的命令. 
VkFence vwrpCreateFence(
    const char*         label,
    VkDevice            device,
    VkFenceCreateInfo*  pCreateInfo    
);


/// @brief 销毁给定的 VkFence.
void vwrpDestroyFence(VkDevice device, VkFence fence);


VkBuffer vwrpCreateBuffer(
    const char*         label,
    VkDevice            device,
    VkBufferCreateInfo* pCreateInfo
);


void vwrpDestroyBuffer(VkDevice device, VkBuffer buffer);


VkDeviceMemory vwrpAllocateDeviceMemory(
    const char*             label,
    VkDevice                device,
    VkMemoryAllocateInfo*   pAllocateInfo
);


void vwrpFreeDeviceMemory(VkDevice device, VkDeviceMemory deviceMemory);


bool vwrpAllocateDescriptorSets(
    VkDevice                        device,
    VkDescriptorSetAllocateInfo*    pAllocateInfo,
    VkDescriptorSet*                outDescriptorSets
);


/// @brief 提交命令缓冲区至队列.
///
/// 注：该函数是外部同步的，因为使用了线程不安全的 `VkQueue` 对象
void vwrpQueueSubmit(
    VkQueue             queue,
    uint32_t            submitCount,
    const VkSubmitInfo  *pSubmitInfos,
    VkFence             fence
);