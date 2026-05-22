#include "renderer_context.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#include "../../common/log.h"
#include "../vulkan/wrapper/queue_family_indices.h"
#include "../vulkan/wrapper/vulkan_wrapper.h"
#include "structs/renderer_context_structs.h"
#include "pipeline_task_record/renderer_context_pipeline_task_record.h"
#include "../data_structs/renderer_data_structs.h"

static bool create_main_render_pass(RendererContext* pContext);
static bool create_swapchain_framebuffers(RendererContext* pContext);
static inline void destroy_swapchain_related_resources(RendererContext* pContext);
static bool create_main_thread_command_pool(RendererContext* pContext);
static inline void destroy_main_thread_command_pool(RendererContext* pContext);
static bool allocate_main_command_buffers(RendererContext* pContext);
static bool create_transfer_command_pool(RendererContext* pContext);
static inline void destroy_transfer_command_pool(RendererContext* pContext);
static bool create_descriptor_set_layouts(RendererContext* pContext);
static inline void destroy_descriptor_set_layouts(RendererContext* pContext);
static bool create_main_render_pass_pipelines(
    RendererContext*                          pContext,
    RctxMainRenderPassPipelinesCreateInfo*    pPipelinesCreateInfo
);
static inline bool create_mainrp_triangle_pipeline(
    RendererContext*           pContext,
    RctxPipelineCreateInfo*    pPipelineCreateInfo,
    RctxPipeline*              pRctxPipeline
);
static inline bool create_mainrp_unlit_pipeline(
    RendererContext*           pContext,
    RctxPipelineCreateInfo*    pPipelineCreateInfo,
    RctxPipeline*              pRctxPipeline
);
static inline void destroy_main_render_pass_pipelines(RendererContext* pContext);
static bool create_descriptor_pools(RendererContext *pContext);
static inline void destroy_descriptor_pools(RendererContext* pContext);
static bool create_sync_objects(RendererContext* pContext);
static inline void destroy_sync_objects(RendererContext* pContext);
static bool record_main_command_buffer(
    RendererContext*            pContext,
    uint32_t                    frameInFlightIndex,
    uint32_t                    swapchainFramebufferIndex,
    MainRenderPassDrawInfo*     pMainRenderPassDrawInfo
);
static void recreate_swapchain(RendererContext* pContext);


RendererContext* rctxNewRendererContext()
{
    // 分配堆内存
    RendererContext* pContext = (RendererContext*)calloc(1, sizeof(RendererContext));
    if (!pContext) return NULL;

    return pContext;
}


bool rctxCreateRendererContext(RendererContext* pContext, GLFWwindow* window)
{
    log_info(ESC_BCOLOR_BRIGHT_BLUE "开始构建渲染器上下文..." ESC_RESET);

    pContext->window = window;                          // 保存窗口句柄

    pContext->instance = vwrpCreateInstance();          // 创建 Vk 实例
    if (pContext->instance == VK_NULL_HANDLE)
        return false;

    log_info("成功创建 Vulkan 实例.");

    pContext->surface = vwrpCreateSurface(pContext->instance, pContext->window); 
    if (pContext->surface == VK_NULL_HANDLE)            // 创建窗口表面 
        return false;
    
    log_info("成功创建窗口表面.");

    pContext->physicalDevice = vwrpPickPhysicalDevice(pContext->instance,
                                   pContext->surface);
    if (pContext->physicalDevice == VK_NULL_HANDLE)     // 选取物理设备
        return false;
    
    vkGetPhysicalDeviceProperties(pContext->physicalDevice,
        &pContext->physicalDeviceProperties);

    log_info("成功选取一个物理设备.");

    pContext->device = vwrpCreateLogicalDevice(pContext->physicalDevice,
                           pContext->surface,                       // 创建 Vk 设备
                           &pContext->graphicsQueueFamilyIndex,
                           &pContext->graphicsQueue,
                           &pContext->presentationQueueFamilyIndex,
                           &pContext->presentationQueue,
                           &pContext->transferQueueFamilyIndex,
                           &pContext->transferQueue);
    if (pContext->device == VK_NULL_HANDLE)
        return false;

    log_info("成功创建逻辑设备.");

    pthread_mutex_init(&pContext->transferMutex, NULL);        // 初始化传输队列的锁 

    pContext->vmaAllocator = vwrpCreateVmaAllocator(pContext->instance,
                                 pContext->physicalDevice,          // 创建 VMA 分配器
                                 pContext->device);
    if (pContext->vmaAllocator == VK_NULL_HANDLE)
        return false;

    log_info("成功创建 VMA 分配器.");

    pContext->swapchain = vwrpCreateSwapchain(pContext->window,
                              pContext->surface,        // 为窗口（表面）创建交换链
                              pContext->physicalDevice,
                              pContext->device,
                              VK_NULL_HANDLE,
                              &pContext->swapchainImageCount,
                              &pContext->swapchainImages,
                              &pContext->swapchainImageFormat,
                              &pContext->swapchainExtent);
    if (pContext->swapchain == VK_NULL_HANDLE)
        return false;

    pContext->swapchainImageViews = vwrpCreateSwapchainImageViews(pContext->device,
                                        pContext->swapchainImageFormat,       
                                        pContext->swapchainImageCount,    // 创建交换链的
                                        pContext->swapchainImages);       // 图像视图
    if (!pContext->swapchainImageViews)
        return false;

    log_info("成功创建交换链.");

    if (!create_main_thread_command_pool(pContext))    // 创建主线程命令池
        return false;

    if (!allocate_main_command_buffers(pContext))      // 分配主命令缓冲区
        return false;

    if (!create_transfer_command_pool(pContext))       // 创建传输用命令池
        return false;

    log_info("成功创建命令池.");

    if (!create_main_render_pass(pContext))            // 创建主渲染通道
        return false;

    log_info("成功创建渲染通道.");

    if(!create_swapchain_framebuffers(pContext))       // 为主渲染通道创建交换链帧缓冲区
        return false;

    log_info("成功创建帧缓冲区.");

    if(!create_descriptor_set_layouts(pContext))
        return false;

    log_info("成功创建描述符集布局.");

    // 为主渲染通道创建所需渲染管线
    RctxMainRenderPassPipelinesCreateInfo mainRenderPassPipelinesCreateInfo = {}; 
    mainRenderPassPipelinesCreateInfo.triangle.vertexSpvFilePath     =
        "./triangle_vert.spv";
    mainRenderPassPipelinesCreateInfo.triangle.vertexSpvEntryPoint   =
        "main";
    mainRenderPassPipelinesCreateInfo.triangle.fragmentSpvFilePath   =
        "./triangle_frag.spv";
    mainRenderPassPipelinesCreateInfo.triangle.fragmentSpvEntryPoint =
        "main";
    mainRenderPassPipelinesCreateInfo.unlit.vertexSpvFilePath     =
        "./unlit_vert.spv";
    mainRenderPassPipelinesCreateInfo.unlit.vertexSpvEntryPoint   =
        "main";
    mainRenderPassPipelinesCreateInfo.unlit.fragmentSpvFilePath   =
        "./unlit_frag.spv";
    mainRenderPassPipelinesCreateInfo.unlit.fragmentSpvEntryPoint =
        "main";

    if (!create_main_render_pass_pipelines(pContext,
             &mainRenderPassPipelinesCreateInfo))
        return false;

    log_info("成功创建渲染管线.");

    if (!create_descriptor_pools(pContext))
        return false;


    if (!create_sync_objects(pContext))     // 创建同步用对象
        return false;


    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)      // 初始化删除列表
    {
        buffer_deletion_list_init(&pContext->bufferDeletionLists[i], 64);
        allocation_deletion_list_init(&pContext->allocationDeletionLists[i], 64);
    }


    log_info(ESC_BCOLOR_BRIGHT_BLUE "渲染器上下文构建完毕." ESC_RESET);

    return true;
}

static bool create_main_thread_command_pool(RendererContext* pContext)
{
    int queueFamilyIndex = -1;
    bool useSingleQueue = 
       has_queue_family_supports_both_graphics_and_presentation(pContext->physicalDevice,
           pContext->surface,
           &queueFamilyIndex);
    
    QueueFamilyIndices queueFamilyIndices = {};
    find_queue_families(pContext->physicalDevice,
        pContext->surface,
        &queueFamilyIndices);

    // 填写 VkCommandPoolCreateInfo
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // RESET_COMMAND_BUFFER_BIT 允许单独重置命令缓冲区
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // 指定命令池对应的队列族，用于绘制命令，故使用含图形队列的队列族
    if (useSingleQueue)
        createInfo.queueFamilyIndex = queueFamilyIndex;
    else
        createInfo.queueFamilyIndex = queueFamilyIndices.graphicsSupport;

    pContext->mainThreadCommandPool = vwrpCreateCommandPool(pContext->device,
                                          &createInfo);
    if (pContext->mainThreadCommandPool == VK_NULL_HANDLE)
        return false;

    return true;
}

static bool allocate_main_command_buffers(RendererContext* pContext)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool        = pContext->mainThreadCommandPool;
    allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    vwrpAllocateCommandBuffers(pContext->device,
        &allocateInfo,
        pContext->mainCommandBuffers);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (pContext->mainCommandBuffers[i] == VK_NULL_HANDLE)
            return false;
    }

    return true;
}

static bool create_transfer_command_pool(RendererContext* pContext)
{
    // 填写 VkCommandPoolCreateInfo
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT 使用一次性的短期命令缓冲区
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    // 指定命令池对应的队列族，用于传输命令，这里即使用传输队列所在的队列族
    createInfo.queueFamilyIndex = pContext->transferQueueFamilyIndex;

    pContext->transferCommandPool = vwrpCreateCommandPool(pContext->device,
                                          &createInfo);
    if (pContext->transferCommandPool == VK_NULL_HANDLE)
        return false;

    return true;
}

static bool create_main_render_pass(RendererContext* pContext)
{
    // 1.填写 VkAttachmentDescription 来描述附件（使用多少个附件就要填多少个）
    // 对于渲染一帧三角形，我们只有一个附件，即（作颜色附件的）交换链中的图像
    // 所以这里只需要填一个该结构体来描述这个附件即可
    VkAttachmentDescription colorAttachment0Description = {};
    // 该附件格式对应为交换链图像的格式
    colorAttachment0Description.format  = pContext->swapchainImageFormat;
    colorAttachment0Description.samples = VK_SAMPLE_COUNT_1_BIT;
    // 渲染通道开始时的 loadOp
    colorAttachment0Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // 渲染通道结束时的 storeOp
    colorAttachment0Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment0Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment0Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // 渲染通道开始前该附件期望的初始布局，因为加载时都会清除，所以可直接描述为未定义
    // 渲染通道结束后该附件需要自动转换的最终布局，我们希望渲染完后将这个附件进行窗口呈现，
    // 故将其描述为呈现源
    colorAttachment0Description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment0Description.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 2.定义子通道
    // 渲染一帧三角形，我们只描述一个子通道即可

    // 首先填写 VkAttachmentReference 来描述该子通道会引用的附件
    // attachment 指定针对 VkAttachmentDescription 的索引值
    // layout 指定该子通道开始时附件需要转化到的布局
    VkAttachmentReference colorAttachment0Reference = {};
    colorAttachment0Reference.attachment = 0;
    colorAttachment0Reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // 注：VkAttachmentReference.layout 引出了布局转换所带来的同步问题，当使用该附件 Ref 的
    // 子通道开始时，Vulkan 会自动对附件根据 layout 字段进行转换.
    // 在这里，我们的渲染通道，即 0 号子通道开始时，首次使用的附件，其布局从
    // Desc.initialLayout 到对应 Ref.layout 的转换会在任意阶段开始 —— 同步问题出现，
    // 倘若使用的附件尚未可用（这里也就是请求交换链图像），则转换操作会发生错误.
    // 一个解决方案是在 vkQueueSubmit 的提交信息里将等待阶段设为
    // VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT，这样整个管线都会等待至图像可用才会开始，但这种方法
    // 简单粗暴，会导致不必要的空闲等待；
    // 另一种方法就是定义内部子通道对外部子通道的依赖（见下文子通道阶段依赖的定义）.

    // 2.5.填写 VkSubpassDescription 来定义子通道
    // 以下代码意为：
    // 该子通道使用图形管线，要使用 colorAttachment0Reference 引用的颜色附件
    VkSubpassDescription subpass0Description = {};
    subpass0Description.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // 绑定颜色附件数组
    // 注：该数组索引值直接对应片段着色器中的代码： 
    // layout(location = 索引值) out vec4 outColorxxx; （用于输出到颜色附件）
    subpass0Description.colorAttachmentCount = 1;
    subpass0Description.pColorAttachments    = &colorAttachment0Reference;

    // 3.定义子通道 `阶段` 依赖
    // 通过填写 VkSubpassDependency 来描述
    // 以下代码意为：
    // dst 子通道的 xx 阶段（在下面指定），要在 src 子通道在 xx 阶段（在下面指定）的执行完毕后
    // 才会执行（以达到控制执行顺序并保证内存同步的目的） 
    VkSubpassDependency subpass0Dependency = {};
    // 源为 VK_SUBPASS_EXTERNAL 特指该 渲染通道 之前的外部操作（隐式子通道）
    subpass0Dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    // 这里指定为颜色附件输出阶段
    subpass0Dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 指定源子通道执行时的内存访问权限，这里只是等待源执行完毕，取 0 为不要求
    subpass0Dependency.srcAccessMask = 0;
    // 目标为该渲染通道的第 0 个子通道（这里也是唯一一个）
    subpass0Dependency.dstSubpass    = 0;
    // 这里意为依赖满足后，目标子通道的颜色附件输出阶段才会开始执行
    subpass0Dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 这里意为目标子通道该阶段需要 写入颜色附件 的内存访问权限
    subpass0Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    

    // 4.创建渲染通道
    // 填写 VkRenderPassCreateInfo
    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;    // 引用 VkAttachmentDescription 数组
    createInfo.pAttachments    = &colorAttachment0Description;
    createInfo.subpassCount    = 1;    // 引用 VkSubpassDescription 数组
    createInfo.pSubpasses      = &subpass0Description;
    createInfo.dependencyCount = 1;    // 应用 VkSubpassDependency 数组
    createInfo.pDependencies   = &subpass0Dependency;

    pContext->mainRenderPass = vwrpCreateRenderPass(pContext->device, &createInfo);
    if (pContext->mainRenderPass == VK_NULL_HANDLE)
        return false;

    return true;
}

static bool create_swapchain_framebuffers(RendererContext* pContext)
{
    // 为帧缓冲区数组分配堆内存
    pContext->swapchainFramebuffers = // 这里需要一个交换链图像对应一个帧缓冲区
        (VkFramebuffer*)calloc(pContext->swapchainImageCount, sizeof(VkFramebuffer));

    // 开始帧缓冲区创建
    for (int i = 0; i < pContext->swapchainImageCount; i++)
    {
        // 指定帧缓冲区的所有附件（通过 VkImageView 指定）
        // 这里只需要交换链图像这一个作颜色附件，故附件数组仅包含一个
        VkImageView attachments[] = {
            pContext->swapchainImageViews[i]
        };

        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // 指定对应 RenderPass
        createInfo.renderPass      = pContext->mainRenderPass;
        // 附件数组信息
        createInfo.attachmentCount = 1;
        createInfo.pAttachments    = attachments;
        // 附件的尺寸（帧缓冲区规定其所有附件都需要为相同尺寸）
        createInfo.width           = pContext->swapchainExtent.width;
        createInfo.height          = pContext->swapchainExtent.height;
        // 需要使用的附件层数（我们的附件（交换链图像）只有一层，所以这里填 1）
        createInfo.layers          = 1;
        
        pContext->swapchainFramebuffers[i] =
            vwrpCreateFramebuffer(pContext->device, &createInfo);
        // 创建失败直接返回 false 退出函数
        if (pContext->swapchainFramebuffers[i] == VK_NULL_HANDLE)
            return false;
    }

    return true;
}

static bool create_descriptor_set_layouts(RendererContext* pContext)
{
    // 1.创建 camera 描述符集布局

    // 填写 VkDescriptorSetLayoutBinding 定义描述符绑定（不是集）
    // （一个描述符 binding 对应一个着色器代码中的 Uniform 变量）
    VkDescriptorSetLayoutBinding cameraUniformBufferLayout = {};
    cameraUniformBufferLayout.binding         = 0;
    // 对于使用动态更新的 Buffer的 描述符集，在绑定集时使用 dynamic offset 就能实现单个
    // buffer 包含多个数据单元的 "uniforms buffer"，故需指定该 binding 类型为
    // UNIFORM_BUFFER_DYNAMIC 即这个描述符指向的是大 buffer，通过 dynamic offset 取里面的
    // uniform 变量数据.
    // (binding 的布局设置只在乎类型和数量（是否数组），不关心具体大小，之后会在真正创建描述符
    // 集时填写 VkDescriptorBufferInfo 来指定)
    cameraUniformBufferLayout.descriptorType  =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    // 这个 descriptorCount 是允许你 binding 的描述符是数组的意思
    cameraUniformBufferLayout.descriptorCount = 1;
    cameraUniformBufferLayout.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT
                                               | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo = {};
    cameraDescriptorSetLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    cameraDescriptorSetLayoutCreateInfo.bindingCount = 1;
    cameraDescriptorSetLayoutCreateInfo.pBindings = &cameraUniformBufferLayout;

    pContext->cameraDescSetLayout.layout =
        vwrpCreateDescriptorSetLayout(pContext->device,
            &cameraDescriptorSetLayoutCreateInfo);
    if (!pContext->cameraDescSetLayout.layout)
        return false;

    // 保存 binding 的布局信息备用
    pContext->cameraDescSetLayout.uniformBufferBindingLayout =
        cameraUniformBufferLayout;

    // 2.创建 drawItems 描述符集布局
    VkDescriptorSetLayoutBinding drawItemsUniformBufferLayout = {};
    drawItemsUniformBufferLayout.binding         = 0;
    drawItemsUniformBufferLayout.descriptorType  = 
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; 
    drawItemsUniformBufferLayout.descriptorCount = 1;
    drawItemsUniformBufferLayout.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT
                                                 | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo drawItemsDescriptorSetLayoutCreateInfo = {};
    drawItemsDescriptorSetLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    drawItemsDescriptorSetLayoutCreateInfo.bindingCount = 1;
    drawItemsDescriptorSetLayoutCreateInfo.pBindings = &drawItemsUniformBufferLayout;

    pContext->drawItemsDescSetLayout.layout =
        vwrpCreateDescriptorSetLayout(pContext->device,
            &drawItemsDescriptorSetLayoutCreateInfo);
    if (!pContext->drawItemsDescSetLayout.layout)
        return false;

    pContext->drawItemsDescSetLayout.uniformBufferBindingLayout =
        drawItemsUniformBufferLayout;

    return true;
}

static bool create_main_render_pass_pipelines(
    RendererContext*                          pContext,
    RctxMainRenderPassPipelinesCreateInfo*    pPipelinesCreateInfo
)
{
    if(!create_mainrp_triangle_pipeline(pContext,
            &pPipelinesCreateInfo->triangle,
            &pContext->mainRenderPassPipelines.triangle))
        return false;

    if(!create_mainrp_unlit_pipeline(pContext,
            &pPipelinesCreateInfo->unlit,
            &pContext->mainRenderPassPipelines.unlit))
        return false;

    return true;
}

static inline bool create_mainrp_triangle_pipeline(
    RendererContext*           pContext,
    RctxPipelineCreateInfo*    pPipelineCreateInfo,
    RctxPipeline*              pRctxPipeline
)
{
    // 0.（配置动态状态）（即不需要静态烘培到管线的状态）
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,      // 视口大小
        VK_DYNAMIC_STATE_SCISSOR        // 剪裁矩形
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates    = dynamicStates;

    // 对于视口和剪裁矩形，由于我们指定它们为动态的状态，故在这只需要指定它们的数量
    // 而不指定具体结构体指针，实际的视口和剪裁矩形将会要在绘制时设置
    VkPipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount  = 1;


    // 1.（配置着色器阶段）创建着色器模块
    // VkShaderModule（SPV 码）在创建图形管线时才会被编译链接成 GPU 机器码，且在管线创建完成
    // 后我们既可以调用 vkDestroyShaderModule 进行销毁也可以存着备后用（比如重建管线之类）
    pRctxPipeline->vertexShaderModule =    // 顶点着色器
        vwrpCreateShaderModule(pContext->device,
            pPipelineCreateInfo->vertexSpvFilePath);
    if (pRctxPipeline->vertexShaderModule == VK_NULL_HANDLE)
        return false;

    pRctxPipeline->fragmentShaderModule =  // 片元着色器
        vwrpCreateShaderModule(pContext->device, 
            pPipelineCreateInfo->fragmentSpvFilePath);
    if (pRctxPipeline->fragmentShaderModule == VK_NULL_HANDLE)
        return false;

    // 1.5.（配置着色器阶段）填写 VkPipelineShaderStageCreateInfo
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};     // 顶点着色器
    vertexShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = pRctxPipeline->vertexShaderModule;
    vertexShaderStageInfo.pName  = pPipelineCreateInfo->vertexSpvEntryPoint;      
                                   // 指定 SPV 的函数入口点
                                   //（SPV 是可以由多个着色器文件编译得来的）

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};   // 片元着色器
    fragmentShaderStageInfo.sType  =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = pRctxPipeline->fragmentShaderModule;
    fragmentShaderStageInfo.pName  = pPipelineCreateInfo->fragmentSpvEntryPoint;

    // 创建 craeteInfo 数组备用
    VkPipelineShaderStageCreateInfo shaderStageInfos[] = {
        vertexShaderStageInfo,
        fragmentShaderStageInfo
    };


    // 2.（配置顶点输入阶段）填写 VkPipelineVertexInputStateCreateInfo
    // 顶点输入阶段需要设置两种结构体，分别是
    // VkVertexInputBindingDescription 和
    // VkVertexInputAttributeDescription

    VkVertexInputBindingDescription vertexBinding0Description = 
        get_vertex_data_input_binding_description(0);

    uint32_t vertexInputAttributeCount = 0;
    get_vertex_data_input_attribute_descriptions(0, &vertexInputAttributeCount, NULL);

    VkVertexInputAttributeDescription 
        vertexInputAttributeDescriptions[vertexInputAttributeCount];
    get_vertex_data_input_attribute_descriptions(0,
        &vertexInputAttributeCount,
        vertexInputAttributeDescriptions);

    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
    vertexInputStateInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // 对于顶点数据用 Buffer 的绑定，这里规定该管线只绑定 1 个 VkBuffer
    vertexInputStateInfo.vertexBindingDescriptionCount   = 1;
    vertexInputStateInfo.pVertexBindingDescriptions      = &vertexBinding0Description;
    vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttributeCount;
    vertexInputStateInfo.pVertexAttributeDescriptions    =
        vertexInputAttributeDescriptions;

    // 把顶点绑定信息记录在管线结构体中备用
    pRctxPipeline->vertexFirstBindingIndex = vertexBinding0Description.binding;
    pRctxPipeline->vertexBindingCount      =
        vertexInputStateInfo.vertexBindingDescriptionCount;

    // 3.（配置输入装配阶段）填写 VkPipelineInputAssemblyStateCreateInfo
    VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyInfo = {};
    vertexInputAssemblyInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vertexInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


    // 4.（配置光栅化阶段）填写 VkPipelineRasterizationStateCreateInfo
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
    rasterizationStateInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;      // 是否禁用光栅化
    rasterizationStateInfo.depthClampEnable        = VK_FALSE;
    rasterizationStateInfo.depthBiasEnable         = VK_FALSE;      // 是否启用深度偏移
    rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateInfo.lineWidth   = 1.0f;
    // 定义顺时针顶点顺序为正面，逆时针顶点顺序为背面
    rasterizationStateInfo.cullMode    = VK_CULL_MODE_BACK_BIT;
    rasterizationStateInfo.frontFace   = VK_FRONT_FACE_CLOCKWISE; 


    // 5.（配置多重采样阶段，MSAA）填写 VkPipelineMultisampleStateCreateInfo
    // 禁用
    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo = {};
    multisamplingStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo.sampleShadingEnable  = VK_FALSE;
    multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


    // 6.（配置深度和模板测试阶段）填写 VkPipelineDepthStencilStateCreateInfo
    // 不需要


    // 7.（配置颜色混合阶段）填写 VkPipelineColorBlendAttachmentState，
    // 其包含针对帧缓冲区中单个颜色附件的颜色混合设置
    // 对于绘制三角形，我们的帧缓冲区仅含一个颜色附件（which is 交换链图像）
    // 所以这里填写一个即可，且不启用混合
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    // 要写入的颜色通道
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
                                              | VK_COLOR_COMPONENT_R_BIT
                                              | VK_COLOR_COMPONENT_G_BIT
                                              | VK_COLOR_COMPONENT_B_BIT;
    colorBlendAttachmentState.blendEnable    = VK_FALSE;

    // 7.5 填写 VkPipelineColorBlendStateCreateInfo, 其包含颜色混合的全局设置（覆盖性的）
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
    colorBlendStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // 是否启用全局覆盖性颜色混合设置，启用会使得上面所有颜色附件的颜色混合设置都被忽略，
    // 全部强制使用逻辑操作计算混合
    colorBlendStateInfo.logicOpEnable   = VK_FALSE;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments    = &colorBlendAttachmentState;


    // 8.（配置 Pipeline Layout）填写 VkPipelineLayoutCreateInfo
    // Pipeline Layout 设置的是 VkDescriptorSetLayout 数组信息和推送常量，与着色器 uniform
    // 有关，该管线不需要任何 Uniform 变量，直接填写 VkPipelineLayoutCreateInfo 即可
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts    = NULL;

    // 8.5.创建管线布局
    pRctxPipeline->pipelineLayout = 
        vwrpCreatePipelineLayout(pContext->device, &pipelineLayoutInfo);
    if (pRctxPipeline->pipelineLayout == VK_NULL_HANDLE)
        return false;

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pDynamicState       = &dynamicStateInfo;
    createInfo.stageCount          = 2;
    createInfo.pStages             = shaderStageInfos;
    createInfo.pVertexInputState   = &vertexInputStateInfo;
    createInfo.pInputAssemblyState = &vertexInputAssemblyInfo;
    createInfo.pViewportState      = &viewportStateInfo;
    createInfo.pRasterizationState = &rasterizationStateInfo;
    createInfo.pMultisampleState   = &multisamplingStateInfo;
    createInfo.pDepthStencilState  = NULL;
    createInfo.pColorBlendState    = &colorBlendStateInfo;
    createInfo.layout              = pRctxPipeline->pipelineLayout;
    createInfo.renderPass          = pContext->mainRenderPass;
    createInfo.subpass             = 0;


    // 9.创建图形管线
    pRctxPipeline->pipeline = vwrpCreateGraphicsPipeline(pContext->device, &createInfo);
    if (pRctxPipeline->pipeline == VK_NULL_HANDLE)
        return false;

    return true;
}

static inline bool create_mainrp_unlit_pipeline(
    RendererContext*           pContext,
    RctxPipelineCreateInfo*    pPipelineCreateInfo,
    RctxPipeline*              pRctxPipeline
)
{
    // 0.（配置动态状态）（即不需要静态烘培到管线的状态）
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,      // 视口大小
        VK_DYNAMIC_STATE_SCISSOR        // 剪裁矩形
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates    = dynamicStates;

    // 对于视口和剪裁矩形，由于我们指定它们为动态的状态，故在这只需要指定它们的数量
    // 而不指定具体结构体指针，实际的视口和剪裁矩形将会要在绘制时设置
    VkPipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount  = 1;


    // 1.（配置着色器阶段）创建着色器模块
    // VkShaderModule（SPV 码）在创建图形管线时才会被编译链接成 GPU 机器码，且在管线创建完成
    // 后我们既可以调用 vkDestroyShaderModule 进行销毁也可以存着备后用（比如重建管线之类）
    pRctxPipeline->vertexShaderModule =    // 顶点着色器
        vwrpCreateShaderModule(pContext->device,
            pPipelineCreateInfo->vertexSpvFilePath);
    if (pRctxPipeline->vertexShaderModule == VK_NULL_HANDLE)
        return false;

    pRctxPipeline->fragmentShaderModule =  // 片元着色器
        vwrpCreateShaderModule(pContext->device, 
            pPipelineCreateInfo->fragmentSpvFilePath);
    if (pRctxPipeline->fragmentShaderModule == VK_NULL_HANDLE)
        return false;

    // 1.5.（配置着色器阶段）填写 VkPipelineShaderStageCreateInfo
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};     // 顶点着色器
    vertexShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = pRctxPipeline->vertexShaderModule;
    vertexShaderStageInfo.pName  = pPipelineCreateInfo->vertexSpvEntryPoint;      
                                   // 指定 SPV 的函数入口点
                                   //（SPV 是可以由多个着色器文件编译得来的）

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};   // 片元着色器
    fragmentShaderStageInfo.sType  =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = pRctxPipeline->fragmentShaderModule;
    fragmentShaderStageInfo.pName  = pPipelineCreateInfo->fragmentSpvEntryPoint;

    // 创建 craeteInfo 数组备用
    VkPipelineShaderStageCreateInfo shaderStageInfos[] = {
        vertexShaderStageInfo,
        fragmentShaderStageInfo
    };


    // 2.（配置顶点输入阶段）填写 VkPipelineVertexInputStateCreateInfo
    // 顶点输入阶段需要设置两种结构体，分别是
    // VkVertexInputBindingDescription 和
    // VkVertexInputAttributeDescription

    VkVertexInputBindingDescription vertexBinding0Description = 
        get_vertex_data_input_binding_description(0);

    uint32_t vertexInputAttributeCount = 0;
    get_vertex_data_input_attribute_descriptions(0, &vertexInputAttributeCount, NULL);

    VkVertexInputAttributeDescription 
        vertexInputAttributeDescriptions[vertexInputAttributeCount];
    get_vertex_data_input_attribute_descriptions(0,
        &vertexInputAttributeCount,
        vertexInputAttributeDescriptions);

    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
    vertexInputStateInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // 对于顶点数据用 Buffer 的绑定，这里规定该管线只绑定 1 个 VkBuffer
    vertexInputStateInfo.vertexBindingDescriptionCount   = 1;
    vertexInputStateInfo.pVertexBindingDescriptions      = &vertexBinding0Description;
    vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttributeCount;
    vertexInputStateInfo.pVertexAttributeDescriptions    =
        vertexInputAttributeDescriptions;

    // 把顶点绑定信息记录在管线结构体中备用
    pRctxPipeline->vertexFirstBindingIndex = vertexBinding0Description.binding;
    pRctxPipeline->vertexBindingCount      =
        vertexInputStateInfo.vertexBindingDescriptionCount;

    // 3.（配置输入装配阶段）填写 VkPipelineInputAssemblyStateCreateInfo
    VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyInfo = {};
    vertexInputAssemblyInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vertexInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


    // 4.（配置光栅化阶段）填写 VkPipelineRasterizationStateCreateInfo
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
    rasterizationStateInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;      // 是否禁用光栅化
    rasterizationStateInfo.depthClampEnable        = VK_FALSE;
    rasterizationStateInfo.depthBiasEnable         = VK_FALSE;      // 是否启用深度偏移
    rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateInfo.lineWidth   = 1.0f;
    // 定义顺时针顶点顺序为正面，逆时针顶点顺序为背面
    rasterizationStateInfo.cullMode    = VK_CULL_MODE_BACK_BIT;
    rasterizationStateInfo.frontFace   = VK_FRONT_FACE_CLOCKWISE; 


    // 5.（配置多重采样阶段，MSAA）填写 VkPipelineMultisampleStateCreateInfo
    // 禁用
    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo = {};
    multisamplingStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo.sampleShadingEnable  = VK_FALSE;
    multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


    // 6.（配置深度和模板测试阶段）填写 VkPipelineDepthStencilStateCreateInfo
    // 暂时不需要


    // 7.（配置颜色混合阶段）填写 VkPipelineColorBlendAttachmentState，
    // 其包含针对帧缓冲区中单个颜色附件的颜色混合设置
    // 这里我们的帧缓冲区仅含一个颜色附件（which is 交换链图像）
    // 所以这里填写一个即可，且不启用混合
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState0 = {};
    // 要写入的颜色通道
    colorBlendAttachmentState0.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
                                               | VK_COLOR_COMPONENT_R_BIT
                                               | VK_COLOR_COMPONENT_G_BIT
                                               | VK_COLOR_COMPONENT_B_BIT;
    colorBlendAttachmentState0.blendEnable    = VK_FALSE;

    // 7.5 填写 VkPipelineColorBlendStateCreateInfo, 其包含颜色混合的全局设置（覆盖性的）
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
    colorBlendStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // 是否启用全局覆盖性颜色混合设置，启用会使得上面所有颜色附件的颜色混合设置都被忽略，
    // 全部强制使用逻辑操作计算混合
    colorBlendStateInfo.logicOpEnable   = VK_FALSE;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments    = &colorBlendAttachmentState0;


    // 8.（配置 Pipeline Layout）填写 VkPipelineLayoutCreateInfo
    // Pipeline Layout 设置的是 VkDescriptorSetLayout 数组信息和推送常量，与着色器 uniform
    // 有关
    VkDescriptorSetLayout descriptorSetLayouts[] =
        { pContext->cameraDescSetLayout.layout,
          pContext->drawItemsDescSetLayout.layout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts    = descriptorSetLayouts;

    // 创建管线布局
    pRctxPipeline->pipelineLayout = 
        vwrpCreatePipelineLayout(pContext->device, &pipelineLayoutInfo);
    if (pRctxPipeline->pipelineLayout == VK_NULL_HANDLE)
        return false;


    // 9.创建图形管线
    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pDynamicState       = &dynamicStateInfo;
    createInfo.stageCount          = 2;
    createInfo.pStages             = shaderStageInfos;
    createInfo.pVertexInputState   = &vertexInputStateInfo;
    createInfo.pInputAssemblyState = &vertexInputAssemblyInfo;
    createInfo.pViewportState      = &viewportStateInfo;
    createInfo.pRasterizationState = &rasterizationStateInfo;
    createInfo.pMultisampleState   = &multisamplingStateInfo;
    createInfo.pDepthStencilState  = NULL;
    createInfo.pColorBlendState    = &colorBlendStateInfo;
    createInfo.layout              = pRctxPipeline->pipelineLayout;
    createInfo.renderPass          = pContext->mainRenderPass;
    createInfo.subpass             = 0;

    pRctxPipeline->pipeline = vwrpCreateGraphicsPipeline(pContext->device, &createInfo);
    if (pRctxPipeline->pipeline == VK_NULL_HANDLE)
        return false;

    return true;
}

// uint32_t cameraDescriptorSetCount, uint32_t drawItemsDescriptorSetCount
static bool create_descriptor_pools(RendererContext *pContext)
{
    // 1. camera Descriptor Set
    
    // camera uniform buffer
    VkDescriptorPoolSize cameraUniformBufferPoolSize = {};
    cameraUniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    cameraUniformBufferPoolSize.descriptorCount = 8;

    VkDescriptorPoolCreateInfo cameraDescriptorSetPoolCreateInfo = {};
    cameraDescriptorSetPoolCreateInfo.sType         =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    cameraDescriptorSetPoolCreateInfo.poolSizeCount = 1;
    cameraDescriptorSetPoolCreateInfo.pPoolSizes    = &cameraUniformBufferPoolSize;
    cameraDescriptorSetPoolCreateInfo.maxSets       = 8;

    pContext->cameraDescSetPool = vwrpCreateDescriptorPool(pContext->device,
                                  &cameraDescriptorSetPoolCreateInfo);
    if (!pContext->cameraDescSetPool)
        return false;

    // 2. drawItems Descriptor Set

    // drawItem(s) uniform buffer
    VkDescriptorPoolSize drawItemUniformBufferPoolSize = {};
    drawItemUniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    drawItemUniformBufferPoolSize.descriptorCount = 16;

    VkDescriptorPoolCreateInfo drawItemsDescriptorSetPoolCreateInfo = {};
    drawItemsDescriptorSetPoolCreateInfo.sType         =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    drawItemsDescriptorSetPoolCreateInfo.poolSizeCount = 1;
    drawItemsDescriptorSetPoolCreateInfo.pPoolSizes    = &drawItemUniformBufferPoolSize;
    drawItemsDescriptorSetPoolCreateInfo.maxSets       = 16;

    pContext->drawItemsDescSetPool = vwrpCreateDescriptorPool(pContext->device,
                                         &drawItemsDescriptorSetPoolCreateInfo);
    if (!pContext->drawItemsDescSetPool)
        return false;

    return true;
}

static bool create_sync_objects(RendererContext* pContext)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // 设置初始状态为已触发，这样第一次等待它时就不会无限阻塞
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        pContext->swapchainImageAvailableSemaphores[i] =
            vwrpCreateSemaphore("for swapchain image available synchronous",
                pContext->device,
                &semaphoreInfo);

        pContext->frameInFlightFences[i] = 
            vwrpCreateFence("for one frame in flight synchronous",
                pContext->device,
                &fenceInfo);

        if (pContext->swapchainImageAvailableSemaphores[i] == VK_NULL_HANDLE
            || pContext->frameInFlightFences[i] == VK_NULL_HANDLE)
            return false;
    }

    // 为呈现用到的信号量分配数组（基于交换链图像数量，所以要动态分配）
    pContext->renderFinishedSemaphores = 
        (VkSemaphore*)calloc(pContext->swapchainImageCount, sizeof(VkSemaphore));

    for (uint32_t i = 0; i < pContext->swapchainImageCount; i++)
    {
        pContext->renderFinishedSemaphores[i] = 
            vwrpCreateSemaphore("for render finished synchronous",
                pContext->device,
                &semaphoreInfo);
        
        if (pContext->renderFinishedSemaphores[i] == VK_NULL_HANDLE)
            return false;
    }

    return true;
}


void rctxDestroyRendererContext(RendererContext* pContext)
{
    log_info(ESC_BCOLOR_BRIGHT_BLUE "销毁渲染器上下文..." ESC_RESET);

    if (!pContext)
    {
        log_info("%s(): 给定渲染器上下文地址无效, 退出.", __func__);
        return;
    }

    if (pContext->instance == VK_NULL_HANDLE)
    {
        log_info("%s(): 给定渲染器上下文不含有效的 VkInstance 句柄，不会销毁任何内容，退出.",
            __func__);
        return;
    }

    vkDeviceWaitIdle(pContext->device);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)      // 释放删除列表
    {
        buffer_deletion_list_release(pContext,
            &pContext->bufferDeletionLists[i]);
        allocation_deletion_list_release(pContext,
            &pContext->allocationDeletionLists[i]);
    }

    /**** 同步对象相关 ****/

    destroy_sync_objects(pContext);

    /**** 管线对象相关 ****/

    destroy_descriptor_pools(pContext);

    destroy_main_render_pass_pipelines(pContext);                  // 销毁管线

    destroy_descriptor_set_layouts(pContext);

    /**** 命令池对象相关 ****/

    destroy_transfer_command_pool(pContext);

    destroy_main_thread_command_pool(pContext);

    /**** 交换链对象相关 ****/

    destroy_swapchain_related_resources(pContext);

    if (pContext->swapchain != VK_NULL_HANDLE)                     // 销毁交换链
        vwrpDestroySwapchain(pContext->device, pContext->swapchain, NULL);
    
    /**** Vk 基础对象相关 ****/

    if (pContext->vmaAllocator != VK_NULL_HANDLE)                  // 销毁 VMA 分配器
        vwrpDestroyVmaAllocator(pContext->vmaAllocator);

    pthread_mutex_destroy(&pContext->transferMutex);               // 销毁传输队列的锁

    if (pContext->device != VK_NULL_HANDLE)                        // 销毁 Vk 设备
        vwrpDestroyLogicalDevice(pContext->device);
    
    if (pContext->surface != VK_NULL_HANDLE)                       // 销毁窗口表面
        vwrpDestroySurface(pContext->instance, pContext->surface);

    vwrpDestroyInstance(pContext->instance);                       // 销毁 Vk 实例

    free(pContext);     // 释放渲染器上下文结构体
    pContext = NULL;    // 占用的内存

    log_info(ESC_BCOLOR_BRIGHT_BLUE "销毁渲染器上下文完毕." ESC_RESET);

    return;
}

static inline void destroy_sync_objects(RendererContext* pContext)
{
    uint32_t i = 0;
    for (; i < pContext->swapchainImageCount; i++)
    {
        if (pContext->renderFinishedSemaphores[i])          // 销毁渲染操作触发用信号量
            vwrpDestroySemaphore(pContext->device,
                pContext->renderFinishedSemaphores[i]);
    }

    free(pContext->renderFinishedSemaphores);   // 释放数组堆内存

    for (i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (pContext->swapchainImageAvailableSemaphores[i]) // 销毁请求图像触发用信号量
            vwrpDestroySemaphore(pContext->device,
                pContext->swapchainImageAvailableSemaphores[i]);

        if (pContext->frameInFlightFences[i])               // 销毁帧栅栏
            vwrpDestroyFence(pContext->device, pContext->frameInFlightFences[i]);
    }
}

static inline void destroy_descriptor_set_layouts(RendererContext* pContext)
{
    if(pContext->cameraDescSetLayout.layout)                  // 销毁描述符集布局
        vwrpDestroyDescriptorSetLayout(pContext->device,
            pContext->cameraDescSetLayout.layout);

    if(pContext->drawItemsDescSetLayout.layout)
        vwrpDestroyDescriptorSetLayout(pContext->device,
            pContext->drawItemsDescSetLayout.layout);
}

static inline void destroy_descriptor_pools(RendererContext* pContext)
{
    if (pContext->cameraDescSetPool)
        vwrpDestroyDescriptorPool(pContext->device,
            pContext->cameraDescSetPool);

    if (pContext->drawItemsDescSetPool)
        vwrpDestroyDescriptorPool(pContext->device,
            pContext->drawItemsDescSetPool);
}

/// @brief 该函数负责销毁 MainRenderPass 所使用的所有管线.
static inline void destroy_main_render_pass_pipelines(RendererContext* pContext)
{
    /**** 销毁 triangle 管线 ****/
    RctxPipeline* pRctxPipeline = &pContext->mainRenderPassPipelines.triangle;

    if (pRctxPipeline->pipeline)
        vwrpDestroyPipeline(pContext->device, pRctxPipeline->pipeline);
    // 销毁管线布局
    if (pRctxPipeline->pipelineLayout)       
        vwrpDestroyPipelineLayout(pContext->device, pRctxPipeline->pipelineLayout);
    // 销毁管线着色器模块
    if (pRctxPipeline->vertexShaderModule)
        vwrpDestroyShaderModule(pContext->device, pRctxPipeline->vertexShaderModule);
    if (pRctxPipeline->fragmentShaderModule)
        vwrpDestroyShaderModule(pContext->device, pRctxPipeline->fragmentShaderModule);

    /**** 销毁 unlit 管线 ****/
    pRctxPipeline = &pContext->mainRenderPassPipelines.unlit;

    if (pRctxPipeline->pipeline)
        vwrpDestroyPipeline(pContext->device, pRctxPipeline->pipeline);
    // 销毁管线布局
    if (pRctxPipeline->pipelineLayout)       
        vwrpDestroyPipelineLayout(pContext->device, pRctxPipeline->pipelineLayout);
    // 销毁管线着色器模块
    if (pRctxPipeline->vertexShaderModule)
        vwrpDestroyShaderModule(pContext->device, pRctxPipeline->vertexShaderModule);
    if (pRctxPipeline->fragmentShaderModule)
        vwrpDestroyShaderModule(pContext->device, pRctxPipeline->fragmentShaderModule);
}

static inline void destroy_transfer_command_pool(RendererContext* pContext)
{
    if (pContext->transferCommandPool)                             // 销毁传输用命令池
        vwrpDestroyCommandPool(pContext->device, pContext->transferCommandPool);
}

static inline void destroy_main_thread_command_pool(RendererContext* pContext)
{
    if (pContext->mainThreadCommandPool)                           // 销毁主线程命令池
        vwrpDestroyCommandPool(pContext->device, pContext->mainThreadCommandPool);
}

/// @brief 该函数会销毁：交换链图像帧缓冲区、渲染通道、交换链图像视图和交换链图像数组.
static inline void destroy_swapchain_related_resources(RendererContext* pContext)
{
    if (pContext->swapchainFramebuffers)            // 销毁交换链帧缓冲区
    {
        for (int i = 0; i < pContext->swapchainImageCount; i++)   
        {
            if (pContext->swapchainFramebuffers[i])
                vwrpDestroyFramebuffer(pContext->device,
                    pContext->swapchainFramebuffers[i]);
        }

        free(pContext->swapchainFramebuffers);      // 释放交换链帧缓冲区数组
        pContext->swapchainFramebuffers = NULL;     // 占用的堆内存
    }

    if (pContext->mainRenderPass)            // 销毁主渲染通道（因为描述了交换链图像做附件）
        vwrpDestroyRenderPass(pContext->device, pContext->mainRenderPass);

    if (pContext->swapchainImageViews)       // 销毁交换链图像视图
        vwrpDestroySwapchainImageViews(pContext->device,
            pContext->swapchainImageCount,
            &pContext->swapchainImageViews);

    if (pContext->swapchainImages)           // 释放交换链图像数组堆内存
    {
        free(pContext->swapchainImages);
        pContext->swapchainImages = NULL;
    }
}


bool rctxCreateStaticBuffer(
    RendererContext*        pContext,
    VkBufferUsageFlagBits   usage,
    size_t                  bufferSize,
    uint64_t                dataOffset,
    size_t                  dataSize,
    const void*             pData,
    VkBuffer*               outBuffer,
    VmaAllocation*          outAllocation
)
{
    if (dataOffset + dataSize > bufferSize)
    {
        log_error("%s(): dataOffset + dataSize 的结果大于 bufferSize！"
            "请检查你的参数填写是否正确！函数返回.", __func__);

        return false;
    }

    // 1.创建 HOST_VISIBLE 的暂存缓冲区
    VkBufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.size        = bufferSize;
    stagingBufferCreateInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    // 该 Buffer 只会在传输队列中使用
    stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo stagingBufferAllocationCreateInfo = {};
    stagingBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingBufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    stagingBufferAllocationCreateInfo.requiredFlags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufferAllocationInfo = {};

    VkResult result = vmaCreateBuffer(pContext->vmaAllocator,
                          &stagingBufferCreateInfo,
                          &stagingBufferAllocationCreateInfo,
                          &stagingBuffer,
                          &stagingAllocation,
                          &stagingBufferAllocationInfo);
    if (result != VK_SUCCESS)
    {
        log_error("%s(): VMA has an error creating VkBuffer! "
            "Error Code(VkResult): %d", __func__, result);

        return false;
    }

    // 将数据写入暂存缓冲区
    uint8_t* address = (uint8_t*)stagingBufferAllocationInfo.pMappedData;
    address += dataOffset;
    memcpy(address, pData, dataSize);

    // 非 coherent 内存，甲方写入后均应手动刷新同步至乙方内存
    vmaFlushAllocation(pContext->vmaAllocator,
        stagingAllocation,
        dataOffset,
        dataSize);

    // 2.接下来创建最终的 Vertex Buffer（DEVICE_LOCAL）
    uint32_t sharingQueueFamilyIndices[2];  // 这里先准备好当资源 Sharing Mode 需为并发时，
                                            // 要用到的队列族指明数组
    VkBufferCreateInfo vertexBufferCreateInfo = {};
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.size  = bufferSize;
    vertexBufferCreateInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    // Buffer 资源会被图形队列和传输队列使用
    if (pContext->graphicsQueueFamilyIndex == pContext->transferQueueFamilyIndex)
    {
        vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        // 并发模式可以让驱动来帮我操心资源的队列所有权转移，我就不用写屏障了
        vertexBufferCreateInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        vertexBufferCreateInfo.queueFamilyIndexCount = 2;
        sharingQueueFamilyIndices[0] = pContext->graphicsQueueFamilyIndex;
        sharingQueueFamilyIndices[1] = pContext->transferQueueFamilyIndex;
        vertexBufferCreateInfo.pQueueFamilyIndices   = sharingQueueFamilyIndices;
    }

    VmaAllocationCreateInfo vertexBufferAllocationCreateInfo = {};
    vertexBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkBuffer staticBuffer = VK_NULL_HANDLE;
    VmaAllocation staticBufferAllocation = VK_NULL_HANDLE;
    result = vmaCreateBuffer(pContext->vmaAllocator,
        &vertexBufferCreateInfo,
        &vertexBufferAllocationCreateInfo,
        &staticBuffer,
        &staticBufferAllocation,
        NULL);
    if (result != VK_SUCCESS)
    {
        log_error("%s(): VMA has an error creating VkBuffer! "
            "Error Code(VkResult): %d", __func__, result);

        vmaDestroyBuffer(pContext->vmaAllocator, stagingBuffer, stagingAllocation);

        return false;
    }

    if (usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    {
        vmaSetAllocationName(pContext->vmaAllocator,
            staticBufferAllocation,
            "VertexBuffer");
    }
    else if (usage == VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        vmaSetAllocationName(pContext->vmaAllocator,
            staticBufferAllocation,
            "IndexBuffer");
    }

    // 3.分配一个传输用命令缓冲区用于执行复制命令
    // 这里是在加锁前准备好需要的结构体和栅栏，避免在锁内分配，减少锁占用时间
    
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool        = pContext->transferCommandPool;
    commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount = 1;

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = dataOffset;  // 源缓冲区偏移量
    copyRegion.dstOffset = dataOffset;  // 目标缓冲区偏移量
    copyRegion.size      = dataSize;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence = vwrpCreateFence("用于 VkBuffer 的复制（Staging 上传）",
                        pContext->device,
                        &fenceCreateInfo);

    // XXX: 这里开始加锁，因为开始使用命令池请求分配 CommandBuffer 了
    pthread_mutex_lock(&pContext->transferMutex);

    vwrpAllocateCommandBuffers(pContext->device,
        &commandBufferAllocInfo,
        &commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    // 4.开始录制这个命令缓冲区
    if (!vwrpBeginCommandBuffer("用于 VkBuffer 的复制（Staging 上传）",
             commandBuffer, 
             &commandBufferBeginInfo))
    {
        pthread_mutex_unlock(&pContext->transferMutex);

        log_error("%s(): 开始录制命令缓冲区时发生错误！上传到设备本地内存失败。",
            __func__);

        vmaDestroyBuffer(pContext->vmaAllocator, stagingBuffer, stagingAllocation);
        vmaDestroyBuffer(pContext->vmaAllocator, staticBuffer, staticBufferAllocation);

        return false;
    }

        vkCmdCopyBuffer(commandBuffer, stagingBuffer, staticBuffer, 1, &copyRegion);

    vwrpEndCommandBuffer("用于 VkBuffer 的复制（Staging 上传）", commandBuffer);

    // 5.可以上传到传输队列了
    vwrpQueueSubmit(pContext->transferQueue, 1, &submitInfo, fence);

    // XXX: 提交到队列后就可以解锁了
    pthread_mutex_unlock(&pContext->transferMutex);

    // 因为锁已经保证了线程安全，故在 C# 端该函数可以包装成异步函数使用，这样就不会阻塞 C# 端
    // 代码；异步方法都会返回一个 Task 供用户查询函数执行状态，这意味着函数的执行完毕应该代表
    // GPU 确实完成了其工作（数据上传、负责、读写等）而不是仅上传了之后函数就退出了。
    // 这样用户就可以根据 Task 的状态来轻松选择执行依赖该函数涉及的资源的代码的时机（比如用户
    // 可以决定加载场景的代码需要等待该场景里模型的数据确实上传 GPU 完毕后才能执行）

    // 6.所以接下来我们不是直接退出函数，而是同步等待 fence
    vkWaitForFences(pContext->device, 1, &fence, VK_TRUE, UINT64_MAX);

    // 命令缓冲区需要执行完毕了才能释放，这里重新加锁
    pthread_mutex_lock(&pContext->transferMutex);

    vkFreeCommandBuffers(pContext->device,
        pContext->transferCommandPool,
        1,
        &commandBuffer);

    pthread_mutex_unlock(&pContext->transferMutex);

    vwrpDestroyFence(pContext->device, fence);

    vmaDestroyBuffer(pContext->vmaAllocator, stagingBuffer, stagingAllocation);

    *outBuffer = staticBuffer;
    *outAllocation = staticBufferAllocation;

    VmaAllocationInfo allocationInfo = {};
    vmaGetAllocationInfo(pContext->vmaAllocator,
        staticBufferAllocation,
        &allocationInfo);

    log_info("创建了一个静态 Buffer %p (name: %s, size: %d).",
        staticBuffer,
        allocationInfo.pName,
        allocationInfo.size);

    return true;
}


bool rctxCreateDynamicBuffer(
    RendererContext*          pContext,
    VkBufferUsageFlagBits     usage,
    size_t                    bufferSize,
    uint64_t                  dataOffset,
    size_t                    dataSize,
    const void*               pData,
    VkBuffer*                 outBuffer,
    VmaAllocation*            outAllocation
)
{
    if (dataOffset + dataSize > bufferSize)
    {
        log_error("%s(): dataOffset + dataSize 的结果大于 bufferSize！"
            "请检查你的参数填写是否正确！函数返回.", __func__);

        return false;
    }

    // dynamic 表示其其处于长期映射状态以方便用户对其进行数据更新.
    // 1.创建缓冲区
    VkBufferCreateInfo dynamicBufferCreateInfo = {};
    dynamicBufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // 对于随时可能进行数据写入更新的动态缓冲区，使用 Ring Buffer 方法创建多个副本空间
    // 这样可以保证不同飞行帧状态之间的数据同步安全（一个飞行帧占用一段副本空间）
    dynamicBufferCreateInfo.size        = bufferSize * MAX_FRAMES_IN_FLIGHT;
    dynamicBufferCreateInfo.usage       = usage;
    // 暂不考虑传输队列，该 Buffer 设计只使用图形队列，CPU 直接更新数据
    dynamicBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo dynamicBufferAllocationCreateInfo = {};
    dynamicBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    // 动态缓冲区设计为持续性映射以及顺序写入友好的，其针对一次写入大量数据进行优化，
    // 故对应的 Buffer 数据更新函数应偏好使用一次大块数据式地全量更新，尽量避免随机写入
    dynamicBufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    // 指明要求内存类型为 HOST_VISIBLE 和 HOST_COHERENT
    dynamicBufferAllocationCreateInfo.requiredFlags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBuffer dynamicBuffer = VK_NULL_HANDLE;
    VmaAllocation dynamicBufferAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo dynamicBufferAllocationInfo = {};

    VkResult result = vmaCreateBuffer(pContext->vmaAllocator,
                          &dynamicBufferCreateInfo,
                          &dynamicBufferAllocationCreateInfo,
                          &dynamicBuffer,
                          &dynamicBufferAllocation,
                          &dynamicBufferAllocationInfo);
    if (result != VK_SUCCESS)
    {
        log_error("%s(): VMA has an error creating VkBuffer! "
            "Error Code(VkResult): %d", __func__, result);

        return false;
    }

    // 将数据写入动态缓冲区（当前飞行帧）
    // 注意，如果从多个线程同时调用该函数，那就必须保证 currentFrameInFlightIndex 在所有调用
    // 的期间值不会被修改，换句话说也就是不能在该函数被调用期间调用 rctxEndFrame()！
    // 下面的 rctxUpdateDynamicBuffer() 也是同理
    uint8_t* address = (uint8_t*)dynamicBufferAllocationInfo.pMappedData;
    address += (bufferSize * pContext->currentFrameInFlightIndex) + dataOffset;
    memcpy(address, pData, dataSize);

    vmaSetAllocationName(pContext->vmaAllocator,
        dynamicBufferAllocation,
        "DynamicBuffer");

    *outBuffer = dynamicBuffer;
    *outAllocation = dynamicBufferAllocation;

    VmaAllocationInfo allocInfo = {};
    vmaGetAllocationInfo(pContext->vmaAllocator, dynamicBufferAllocation, &allocInfo);

    log_info("创建了一个动态 Buffer %p (name: %s, size: %d).",
        dynamicBuffer,
        allocInfo.pName,
        allocInfo.size);

    return true;
}


void rctxUpdateDynamicBuffer(
    RendererContext*          pContext,
    size_t                    bufferSize,
    uint32_t                  dataOffset,
    size_t                    dataSize,
    const void*               pData,
    VmaAllocation             allocation
)
{
    if (dataOffset + dataSize > bufferSize)
    {
        log_error("%s(): dataOffset + dataSize 的结果大于 bufferSize！"
            "请检查你的参数填写是否正确！函数返回.", __func__);

        return;
    }

    VmaAllocationInfo allocationInfo = {};
    vmaGetAllocationInfo(pContext->vmaAllocator,
        allocation,
        &allocationInfo);

    // 将数据写入动态缓冲区（应用到当前飞行帧副本）
    uint8_t* address = (uint8_t*)allocationInfo.pMappedData;
    address += (bufferSize * pContext->currentFrameInFlightIndex) + dataOffset;
    memcpy(address, pData, dataSize);
}


bool rctxRequestDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
)
{
    // 延迟销毁（添加句柄到当前飞行帧对应的 DeletionList 中去）
    if (!deletion_list_add(&pContext->
            bufferDeletionLists[pContext->currentFrameInFlightIndex].base, buffer))
        return false;

    if (!deletion_list_add(&pContext->
            allocationDeletionLists[pContext->currentFrameInFlightIndex].base,
            allocation))
        return false;

    return true;
}


void rctxWaitIdle(RendererContext* pContext)
{
    vkDeviceWaitIdle(pContext->device);
}


void rctxDestroyBuffer(
    RendererContext*    pContext,
    VkBuffer            buffer,
    VmaAllocation       allocation
)
{
    log_info("立即销毁 Buffer: %p.", buffer);

    vmaDestroyBuffer(pContext->vmaAllocator, buffer, allocation);
}


bool rctxCreateCameraDescriptorSet(
    RendererContext*                pContext,
    size_t                          bufferOffset,
    size_t                          bufferRange,
    VkBuffer                        uniformBuffer,
    VkDescriptorSet*                outDescriptorSet
)
{
    RctxCameraDescriptorSetLayout setLayoutInfo = pContext->cameraDescSetLayout;

    // 分配描述符集
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool     = pContext->cameraDescSetPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts        = &setLayoutInfo.layout;

    if (!vwrpAllocateDescriptorSets(pContext->device, &allocateInfo, outDescriptorSet))
        return false;

    // 配置描述符集中的描述符，这里 camera descriptor set 只有一个描述符 binding，
    // 为 uniform buffer（binding 0，见 pContext->cameraSetLayout）
    VkDescriptorBufferInfo uniformBufferInfo = {};
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = bufferOffset;
    uniformBufferInfo.range  = bufferRange;

    VkWriteDescriptorSet setWrite = {};
    setWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    setWrite.dstSet          = *outDescriptorSet;
    setWrite.dstBinding      = setLayoutInfo.uniformBufferBindingLayout.binding;
    setWrite.descriptorType  = setLayoutInfo.uniformBufferBindingLayout.descriptorType;
    // 当描述符 binding 是数组形式时，设置要写入的对应索引
    setWrite.dstArrayElement = 0;
    setWrite.descriptorCount = setLayoutInfo.uniformBufferBindingLayout.descriptorCount;
    // pBufferInfo 的数量 = descriptorCount - dstArrayElement，用于数组形式的 binding 更新
    setWrite.pBufferInfo     = &uniformBufferInfo;

    vkUpdateDescriptorSets(pContext->device, 1, &setWrite, 0, NULL);

    return true;
}


bool rctxCreateDrawItemsDescriptorSet(
    RendererContext*                pContext,
    size_t                          bufferOffset,
    size_t                          bufferRange,
    VkBuffer                        uniformBuffer,
    VkDescriptorSet*                outDescriptorSet
)
{
    RctxDrawItemsDescriptorSetLayout setLayoutInfo = pContext->drawItemsDescSetLayout;

    // 分配描述符集
    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool     = pContext->drawItemsDescSetPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts        = &setLayoutInfo.layout;

    if (!vwrpAllocateDescriptorSets(pContext->device, &allocateInfo, outDescriptorSet))
        return false;

    // 配置描述符集中的描述符，这里 drawItems descriptor set 只有一个描述符 binding，
    // 为 uniform buffer (binding 0)
    VkDescriptorBufferInfo uniformBufferInfo = {};
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = bufferOffset;
    uniformBufferInfo.range  = bufferRange;

    VkWriteDescriptorSet setWrite = {};
    setWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    setWrite.dstSet          = *outDescriptorSet;
    setWrite.dstBinding      = setLayoutInfo.uniformBufferBindingLayout.binding;
    setWrite.descriptorType  = setLayoutInfo.uniformBufferBindingLayout.descriptorType;
    // 当描述符 binding 是数组形式时，设置要写入的对应索引
    setWrite.dstArrayElement = 0;
    setWrite.descriptorCount = setLayoutInfo.uniformBufferBindingLayout.descriptorCount;
    // pBufferInfo 的数量 = descriptorCount - dstArrayElement，用于数组形式的 binding 更新
    setWrite.pBufferInfo     = &uniformBufferInfo;

    vkUpdateDescriptorSets(pContext->device, 1, &setWrite, 0, NULL);

    return true;
}


void rctxBeginFrame(RendererContext* pContext)
{
    // 0.等待当前飞行帧栅栏
    vkWaitForFences(pContext->device,
        1,
        &pContext->frameInFlightFences[pContext->currentFrameInFlightIndex],
        VK_TRUE,
        UINT64_MAX);

    // 0.5.对当前飞行帧的删除队列进行销毁刷新
    deletion_list_flush(pContext,
        &pContext->bufferDeletionLists[pContext->currentFrameInFlightIndex].base);
    deletion_list_flush(pContext,
        &pContext->allocationDeletionLists[pContext->currentFrameInFlightIndex].base);
}


void rctxDrawFrame(
    RendererContext*                     pContext,
    bool                                 isFramebufferResized,
    // MainRenderPassPipelinesDrawInfo*     pMainRenderPassPipelinesDrawInfo,
    MainRenderPassDrawInfo*              pMainRenderPassDrawInfo
)
{
    // 检查传入参数
    if (!pMainRenderPassDrawInfo)
    {
        log_error("%s(): 传入了无效的 "
            "pMainRenderPassDrawInfo (MainRenderPassDrawInfo*)，"
            "该参数不得为 NULL！", __func__);

        return;
    }
    // 设置 record 函数指针供后续主命令录制时使用   /*, ...RenderPassDrawInfo...*/
    set_pipeline_task_record_func(pMainRenderPassDrawInfo);

    // 防止获取变量的代码过长
    static uint32_t frameInFlightIndex = 0;

    frameInFlightIndex = pContext->currentFrameInFlightIndex;

    // 1.请求交换链图像（获取可用交换链图像索引）
    uint32_t swapchainImageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(pContext->device,
                        pContext->swapchain,
                        UINT64_MAX,
                        pContext->swapchainImageAvailableSemaphores[frameInFlightIndex],
                        VK_NULL_HANDLE,
                        &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)   // 过时的交换链与表面不再兼容，不再可用于渲染,
    {                                         // 我们需要重建交换链；若为次优我们仍可对已请
        recreate_swapchain(pContext);         // 求的图像继续渲染，等提交其至呈现后我们再重
                                              // 建交换链.
        return;     // 返回，直接跳过这一次的 draw_frame 调用
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        log_error("%s()：请求交换链图像发生错误！Error Code(VkResult)：%d",
            __func__, result);

        return;     // 呃这样应该没问题？...
    }

    // 等待完毕并检查不需要交换链重建后重置 In Flight 帧栅栏
    //（确保下文我们一定会使用该帧栅栏如调用 vkQueueSubmit() 时，才重置，不然的话如果在检查
    // 交换链是否需要重建之前就重置帧栅栏，当遇到交换链需要重建时 draw_frame 会直接 return，
    // 这样在该次 draw_frame 被重置的栅栏就没有任何提交函数会使用和触发它，而下一次等待它的
    // draw_frame 就会永远等待它，造成死锁）
    vkResetFences(pContext->device,
        1,
        &pContext->frameInFlightFences[frameInFlightIndex]);
    // 现在可以安全处理该帧的命令缓冲区了

    // 2.重置主命令缓冲区
    vkResetCommandBuffer(pContext->mainCommandBuffers[frameInFlightIndex], 0);

    // 3.录制主命令缓冲区，传入对应交换链图像索引（也对应帧缓冲区索引）
    record_main_command_buffer(pContext,
        frameInFlightIndex,
        swapchainImageIndex,
        pMainRenderPassDrawInfo);
    
    // 4.准备提交主命令缓冲区
    
    // 指定提交要等待的所有信号量（这里只有一个信号量需要等待，即 交换链图像可用 信号量）
    VkSemaphore semaphoresToWait[] = {
        pContext->swapchainImageAvailableSemaphores[frameInFlightIndex]
    };

    // 指定所有等待信号量对应的等待阶段
    // “这里我们直到将要输出到颜色附件阶段时才会用到交换链图像，所以在此阶段等待交换链图像可用
    // 信号量” —— 说法是错误的，实际上在渲染通道开始时，由于没有定义外部子通道依赖，隐式的外部
    // 子通道依赖会使得引用图像的子通道（这里是引用了交换链图像）会在任何可能的时间点对这些图像
    // 进行布局转换（Desc.initLayout 到 Ref.layout），所以从这个方面看，这里的 waitOnStages
    // 应该是 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT 阶段才对，但这样设置会导致不必要的空闲等待，
    // 毕竟在这里我们子通道前期阶段的执行确实不会用到交换链图像；所以更好的方法是显式定义与外部
    // 子通道的子通道依赖，告诉 Vulkan 我们该子通道的
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 阶段（真正开始使用交换链图像的阶段）
    // 依赖于 VK_SUBPASS_EXTERNAL（特殊值，指代隐式的外部子通道）的
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 阶段 —— 这个阶段的完成意味着请求的交
    // 换链图像可用）
    VkPipelineStageFlags waitOnStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    
    // 指定命令缓冲区执行完毕后要触发的所有信号量（这里指定触发 渲染完毕 信号量）
    VkSemaphore semaphoresToSignal[] = {
        pContext->renderFinishedSemaphores[swapchainImageIndex]
    };

    // 填写提交信息
    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = semaphoresToWait;
    submitInfo.pWaitDstStageMask    = waitOnStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = 
        &pContext->mainCommandBuffers[frameInFlightIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = semaphoresToSignal;

    // 提交命令缓冲区至图形队列执行，同时给定栅栏供执行完毕后触发
    vwrpQueueSubmit(pContext->graphicsQueue,
        1,
        &submitInfo,
        pContext->frameInFlightFences[frameInFlightIndex]);

    // 5.准备呈现渲染完毕的交换链图像
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = semaphoresToSignal;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &pContext->swapchain;
    presentInfo.pImageIndices      = &swapchainImageIndex;

    // 真的、真的... 绕了好大一圈的远路啊......

    // 呈现
    // 注：这里必须说明的是，信号量是 “被消耗的”，一旦其被触发，谁等待它，谁就负责重置它
    // 所以这里提交了呈现，进入下一个循环时，假设上个循环的呈现操作不够快，还没来得及 “消耗” 
    // 指定的 presentInfo.pWaitSemaphores 中的信号量时，这些信号量在下个循环，甚至是下个循
    // 环执行到提交渲染操作（调用 vkQueueSubmit）、GPU 执行渲染操作完毕，将要触发指定要触发
    // 的信号量（也就是呈现要等待的信号量）时，它们都还未被上个循环的呈现操作消耗掉（仍处于已
    // 触发状态），这便导致了验证层 VUID-vkQueueSubmit-pSignalSemaphores-00067 报错.
    // 
    // 简单来说，vkQueuePresentKHR() 不像 vkQueueSubmit()，前者并没有提供触发信号量或栅栏之
    // 类的同步原语的方法 (在没有扩展的情况下)，所以重用呈现操作所消耗的信号量对象的时机并不太
    // 清晰，详见 https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html.
    // 推荐的解决方案是，基于交换链图像的数量创建多个信号量，每次 InFlight 循环所用的交换链图
    // 像其渲染和呈现操作均使用 swapchainImageIndex 来取信号量使用，一个交换链图像对应一个信
    // 号量，这样上个循环使用的信号量就不关下个循环的事，等到下一次使用同一个信号量时，消耗它
    // 的呈现操作必然是早就完成了的.
    result = vkQueuePresentKHR(pContext->presentationQueue, &presentInfo);
    // 如果交换链过时或为次优，重建交换链
    if (result == VK_ERROR_OUT_OF_DATE_KHR 
        || result == VK_SUBOPTIMAL_KHR
        || isFramebufferResized)
        recreate_swapchain(pContext);
    else if (result != VK_SUCCESS)
        log_error("%s()：交换链图像的呈现发生错误！Error Code(VkResult)：%d",
            __func__, result);

}

/// @brief 根据传入的已分类的绘制信息，按渲染通道依次录制子通道（绑定、绘制），该函数的录制包含
/// 整一帧的所有工作
///
/// @param frameInFlightIndex 可用的主命令缓冲区索引（作录制目标）
/// @param swapchainFramebufferIndex 可用的交换链帧缓冲区索引（作渲染目标）
/// @param pMainRenderPassPipelinesDrawInfo 主 RenderPass 的所有管线绘制信息
///
/// @return 发生错误时返回 `false`
static bool record_main_command_buffer(
    RendererContext*            pContext,
    uint32_t                    frameInFlightIndex,
    uint32_t                    swapchainFramebufferIndex,
//  TODO: 设计某种 PipelinesDrawInfo，让调用者以渲染通道为单位，传入必要子通道绘制信息，例如：
    MainRenderPassDrawInfo*     pMainRenderPassDrawInfo
)
{
    char* label = "main command buffer";

    VkCommandBuffer commandBuffer = 
        pContext->mainCommandBuffers[frameInFlightIndex];

    // 1.开始录制命令缓冲区
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = NULL;

    if (!vwrpBeginCommandBuffer(label, commandBuffer, &commandBufferBeginInfo))
        return false;

    // 2.开始录制 MainRenderPass
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass        = pContext->mainRenderPass;
    renderPassBeginInfo.framebuffer       = pContext->
        swapchainFramebuffers[swapchainFramebufferIndex];
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassBeginInfo.renderArea.extent = pContext->swapchainExtent;
    // 黑色清屏
    VkClearValue clearValue = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassBeginInfo.clearValueCount   = 1;
    renderPassBeginInfo.pClearValues      = &clearValue;

    // 3.进入渲染通道实例
    // 注：调用 vkCmdBeginRenderPass() 会隐式开始指定 RenderPass 的第 0 个 subpass
    // 这里是 Main RenderPass，Default subpass
    vkCmdBeginRenderPass(commandBuffer,
        &renderPassBeginInfo,
        VK_SUBPASS_CONTENTS_INLINE);

        // 为 Default subpass 进行管线绑定、绘制命令录制
        SubpassDrawInfo* pSubpassDrawInfo = &pMainRenderPassDrawInfo->defaultPass;
        // pMainRenderPassDrawInfo->defaultPass.pipelineDrawTaskCount 指明了这次
        // DrawFrame 为 default subpass 使用的管线数量，
        // pMainRenderPassDrawInfo->defaultPass.pipelineDrawTasks 便是具体的管线的绘制信
        // 息所在处，即一个 PipelineDrawTask 包含了 “一种” 管线的所有绘制信息.
        for(int i = 0;
            i < pSubpassDrawInfo->pipelineDrawTaskCount;
            i++)
        {
            pSubpassDrawInfo->pipelineDrawTasks[i].record(pContext,
                commandBuffer,
                pSubpassDrawInfo->pipelineDrawTasks[i].pPipelineDrawInfo);

            // XXX:
            // 可以看到这里根本不关心使用的是什么管线，并没有进行硬编码，这使得未来管线的增添
            // 变得容易. 相较于 Pipeline，RenderPass 的设计就较为稳定不多变，所以这里设计外
            // 部传入的绘制信息的粗细度是以 Subpass Draw Info 为准，而不是以某某
            // Pipeline Draw Info 硬编码式地传入.
        }

    // 对于目前的 mainRenderPass，我们只有一个 subpass（直接绘制到颜色附件），
    // 所以现在可以结束 RenderPass 了
    //（对于多个 subpass 的 RenderPass，我们要按对应 RenderPass 的设计，调用
    // vkCmdNextSubpass() 以表示进入下一个 subpass）

    // 4.结束录制渲染通道
    vkCmdEndRenderPass(commandBuffer);
    
    // 我们目前只有一个 mainRenderPass，所以现在可以结束录制命令缓冲区了
    //（对于多个 RenderPass，我们调用 vkCmdBeginRenderPass() 以录制下一个 RenderPass）

    // 5.结束录制命令缓冲区
    if (!vwrpEndCommandBuffer(label, commandBuffer))
        return false;

    return true;
}

static void recreate_swapchain(RendererContext* pContext)
{
    // 首先获取 GLFW 窗口 Framebuffer 大小，检查若有分量等于 0 的则返回，不做交换链的重建
    int width = 0, height = 0;
    glfwGetFramebufferSize(pContext->window, &width, &height);
    if (width == 0 || height == 0)
        return;

    log_info("触发了交换链重建，Old Swapchain: %p, extent: %d x %d",
        pContext->swapchain,
        pContext->swapchainExtent.width,
        pContext->swapchainExtent.height);

    vkDeviceWaitIdle(pContext->device);

    // 在重新创建交换链和相关对象前，先销毁旧交换链相关的对象
    destroy_swapchain_related_resources(pContext);

    // 引用为旧交换链
    VkSwapchainKHR oldSwapchain = pContext->swapchain;

    // 重新创建交换链
    pContext->swapchain = vwrpCreateSwapchain(pContext->window,
                              pContext->surface,
                              pContext->physicalDevice,
                              pContext->device,
                              oldSwapchain,
                              &pContext->swapchainImageCount,
                              &pContext->swapchainImages,
                              &pContext->swapchainImageFormat,
                              &pContext->swapchainExtent);

    // 销毁旧交换链
    if (oldSwapchain != VK_NULL_HANDLE)
        vwrpDestroySwapchain(pContext->device, oldSwapchain, NULL);

    // 重新创建交换链图像视图
    pContext->swapchainImageViews = vwrpCreateSwapchainImageViews(pContext->device,
                                        pContext->swapchainImageFormat,
                                        pContext->swapchainImageCount,
                                        pContext->swapchainImages);

    // 重新创建给交换链图像用的渲染通道（因为图像格式可能变化）
    create_main_render_pass(pContext);

    // 重新创建交换链帧缓冲区（因为其直接引用交换链图像）
    create_swapchain_framebuffers(pContext);

    log_info("交换链重建完毕，New Swapchain: %p, extent: %d x %d",
        pContext->swapchain,
        pContext->swapchainExtent.width,
        pContext->swapchainExtent.height);
}


void rctxEndFrame(RendererContext* pContext)
{
    // 6.切换到下一飞行帧
    pContext->currentFrameInFlightIndex =
        (pContext->currentFrameInFlightIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

