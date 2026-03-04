#pragma once

#include "../common/log.h"
#include "queue_family_indices.h"
#include "swapchain_support_details.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


/// @brief 创建 VkInstance，其是程序和 Vulkan 库之间的接口.
/// 
/// @return 返回新创建的 VkInstance 句柄（当发生错误时返回 `NULL`）
VkInstance createInstance(void);


/// @brief 销毁给定的 VkInstance.
void destroyInstance(VkInstance instance);


/// @brief 在成功创建 VkInstance 后调用该函数创建 VkSurfaceKHR，其是对窗口系统中
/// 具体窗口对象 的抽象.
///
/// @param instance 调用该函数需要传入一个有效的 VkInstance 句柄
/// （需确保已启用了 `VK_KHR_surface` 扩展和平台相关的扩展如 `VK_KHR_win32_surface`）
/// @param window 调用该函数需要传入一个有效的 GLFWwindow 句柄
///
/// @return 返回新创建的 VkSurfaceKHR 句柄（当发生错误时返回 `NULL`）
VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);


/// @brief 销毁给定的 VkSurfaceKHR.
///
/// @param instance 调用该函数需要传入一个对应的 VkInstance 句柄
void destroySurface(VkInstance instance, VkSurfaceKHR surface);


/// @brief 查询可用物理设备并尝试选择可用的显卡作 PhysicalDevice.
///
/// @param instance 调用该函数需要传入一个有效的 VkInstance 句柄
/// @param surface 调用该函数需要传入一个有效的 VkSurfaceKHR 句柄
///
/// @return 返回一个可用的 PhysicalDevice 句柄（当发生错误时返回 `NULL`）
VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);


/// @brief 根据给定物理设备创建逻辑设备.
///
/// @param graphicsQueue 函数执行成功后，该参数会接收一个新的 VkQueue 句柄（graphics）
/// @param presentationQueue 函数执行成功后，该参数会接收一个新的 VkQueue 句柄（presentation）
///
/// @return 返回新创建的 VkDevice 句柄（当发生错误时返回 `NULL`）
VkDevice createLogicalDevice(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    VkQueue*            graphicsQueue,
    VkQueue*            presentationQueue
);


/// @brief 销毁给定的 VkDevice.
void destroyLogicalDevice(VkDevice device);


/// @brief 根据给定窗口句柄和设备创建交换链.
///
/// @param window 给定窗口句柄
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
VkSwapchainKHR createSwapchain(
    GLFWwindow*         window,
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
/// @param ppSwapchainImages 需同时传入在 createSwapchain() 中分配的交换链图像数组供销毁
/// (可传入 `NULL`，这时函数不会帮你销毁交换链图像数组，需要你自己销毁，例如你在重建交换链，
/// 需分开销毁交换链和其图像数组)
void destroySwapchain(
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
VkImageView* createSwapchainImageViews(
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
void destroySwapchainImageViews(
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
VkCommandPool createCommandPool(
    VkDevice                        device,
    const VkCommandPoolCreateInfo*  pCreateInfo
);


/// @brief 销毁给定的 VkCommandPool.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param commandPool 要销毁的命令池句柄
void destroyCommandPool(VkDevice device, VkCommandPool commandPool);


/// @brief 分配（多个）命令缓冲区.
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pAllocateInfo 给定的命令缓冲区分配信息
/// @param pCommandBuffers 输出参数作为目标，要求指向有效的 VkCommandBuffer 句柄的数组
void allocateCommandBuffers(
    VkDevice                            device,
    const VkCommandBufferAllocateInfo*  pAllocateInfo,
    VkCommandBuffer*                    pCommandBuffers
);


/// @brief 开始录制给定命令缓冲区.
///
/// @param label 描述性标签，仅供输出日志信息用
/// @param commandBuffer 目标命令缓冲区
/// @param pBeginInfo 给定的开始录制信息
///
/// @return 函数执行成功返回 `true`，反之遇到错误返回 `false`
bool beginCommandBuffer(
    const char*                     label,
    VkCommandBuffer                 commandBuffer,
    const VkCommandBufferBeginInfo  *pBeginInfo
);


/// @brief 结束录制给定命令缓冲区.
///
/// @param label 描述性标签，仅供输出日志信息用
/// @param commandBuffer 目标命令缓冲区
///
/// @return 函数执行成功返回 `true`，反之遇到错误返回 `false`
bool endCommandBuffer(const char* label, VkCommandBuffer commandBuffer);


/// @brief 创建渲染通道，其用于为管线提供目标帧缓冲区中关于附件的信息.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的渲染通道创建信息
///
/// @return 返回新创建的 VkRenderPass 句柄（当发生错误时返回 `NULL`）
VkRenderPass createRenderPass(
    VkDevice                        device,
    const VkRenderPassCreateInfo*   pCreateInfo
);


/// @brief 销毁给定的 VkRenderPass.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param renderPass 要销毁的渲染通道句柄
void destroyRenderPass(VkDevice device, VkRenderPass renderPass);


/// @brief 创建帧缓冲区，其用作管线渲染的目标，帧缓冲区包含了具体的附件
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的帧缓冲区创建信息
///
/// @return 返回新创建的 VkFramebuffer 句柄（当发生错误时返回 `NULL`）
VkFramebuffer createFramebuffer(VkDevice device, VkFramebufferCreateInfo* pCreateInfo);


/// @brief 销毁给定的 VkFramebuffer.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param framebuffer 要销毁的帧缓冲区句柄
void destroyFramebuffer(VkDevice device, VkFramebuffer framebuffer);


/// @brief 创建着色器模块，其用于为管线提供着色器阶段的着色器代码源.
///
/// @param device 调用该函数需要传入对应的 VkDevice 句柄
/// @param spvFilePath 对应 SPIR-V 文件的目录
///
/// @return 返回新创建的 VkShaderModule 句柄（当发生错误时返回 `NULL`）
VkShaderModule createShaderModule(
    VkDevice        device,
    const char*     spvFilePath
);


/// @brief 销毁给定的 VkShaderModule.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param shaderModule 要销毁的着色器模块句柄
void destroyShaderModule(VkDevice device, VkShaderModule shaderModule);


/// @brief 创建管线布局，其用于为管线提供描述符集的信息.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的管线布局创建信息
///
/// @return 返回新创建的 VkPipelineLayout 句柄（当发生错误时返回 `NULL`）
VkPipelineLayout createPipelineLayout(
    VkDevice                            device,
    const VkPipelineLayoutCreateInfo*   pCreateInfo
);


/// @brief 销毁给定的 VkPipelineLayout.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pipelineLayout 要销毁的管线布局句柄
void destroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout);


/// @brief 创建图形管线.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pCreateInfo 给定的管线创建信息
///
/// @return 返回新创建的 VkPipeline 句柄（当发生错误时返回 `NULL`）
VkPipeline createGraphicsPipeline(
    VkDevice                             device,
    const VkGraphicsPipelineCreateInfo*  pCreateInfo
);


/// @brief 销毁给定的 VkPipeline.
///
/// @param device 调用该函数需要传入一个对应的 VkDevice 句柄
/// @param pipeline 要销毁的管线句柄
void destroyPipeline(VkDevice device, VkPipeline pipeline);


VkSemaphore createSemaphore(
    const char*             label,
    VkDevice                device,
    VkSemaphoreCreateInfo*  pCreateInfo    
);


void destroySemaphore(VkDevice device, VkSemaphore semaphore);


VkFence createFence(
    const char*         label,
    VkDevice            device,
    VkFenceCreateInfo*  pCreateInfo    
);


void destroyFence(VkDevice device, VkFence fence);


VkBuffer createBuffer(
    const char*         label,
    VkDevice            device,
    VkBufferCreateInfo* pCreateInfo
);


void destroyBuffer(VkDevice device, VkBuffer buffer);


VkDeviceMemory allocateDeviceMemory(
    const char*             label,
    VkDevice                device,
    VkMemoryAllocateInfo*   pAllocateInfo
);


void freeDeviceMemory(VkDevice device, VkDeviceMemory deviceMemory);

void queueSubmit(
    VkQueue             queue,
    uint32_t            submitCount,
    const VkSubmitInfo  *pSubmitInfos,
    VkFence             fence
);