#include "renderer_context.h"

static bool create_main_thread_command_pool(RendererContext* pContext);
static bool allocate_main_command_buffers(RendererContext* pContext);
static bool create_main_render_pass(RendererContext* pContext);
static bool create_swapchain_framebuffers(RendererContext* pContext);
static bool create_main_render_pass_pipeline_0(
    RendererContext*    pContext,
    const char*         vertexSpvFilePath,
    const char*         vertexSpvEntryPoint,
    const char*         fragmentSpvFilePath,
    const char*         fragmentSpvEntryPoint
);
static bool create_sync_objects(RendererContext* pContext);
static uint32_t get_device_memory_type_index(
    VkPhysicalDeviceMemoryProperties*   pPhysicalDeviceMemoryProperties,
    uint32_t                            requiredMemoryTypeBits,
    VkMemoryPropertyFlags               requiredMemoryPropertyFlags
);
static bool record_main_command_buffer(
    RendererContext*                    pContext,
    uint32_t                            frameInFlightIndex,
    uint32_t                            swapchainFramebufferIndex,
    MainRenderPassPipeline0DrawInfo*    pMainRenderPassPipeline0DrawInfo
);
static void destroy_swapchain_related_resources(RendererContext* pContext);
static void recreate_swapchain(RendererContext* pContext);


RendererContext* rctxNewRendererContext()
{
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif

    // 分配堆内存
    RendererContext* pContext = (RendererContext*)calloc(1, sizeof(RendererContext));
    if (!pContext) return NULL;

    return pContext;
}


bool rctxCreateRendererContext(RendererContext* pContext, GLFWwindow* window)
{
    log_info("开始构建渲染器上下文...");

    pContext->window = window;                          // 保存窗口句柄

    pContext->instance = createInstance();              // 创建 Vk 实例
    if (pContext->instance == VK_NULL_HANDLE)
        return false;

    pContext->surface = createSurface(pContext->instance, pContext->window); 
    if (pContext->surface == VK_NULL_HANDLE)            // 创建窗口表面 
        return false;
    
    pContext->physicalDevice = pickPhysicalDevice(pContext->instance, pContext->surface);
    if (pContext->physicalDevice == VK_NULL_HANDLE)     // 选取物理设备
        return false;
    
    pContext->device = createLogicalDevice(pContext->physicalDevice,    // 创建 Vk 设备
                           pContext->surface,
                           &pContext->graphicsQueue,
                           &pContext->presentationQueue);
    if (pContext->device == VK_NULL_HANDLE)
        return false;

    pContext->swapchain = createSwapchain(pContext->window,    // 为窗口（表面）创建交换链
                              pContext->surface,
                              pContext->physicalDevice,
                              pContext->device,
                              VK_NULL_HANDLE,
                              &pContext->swapchainImageCount,
                              &pContext->swapchainImages,
                              &pContext->swapchainImageFormat,
                              &pContext->swapchainExtent);
    if (pContext->swapchain == VK_NULL_HANDLE)
        return false;

    pContext->swapchainImageViews = createSwapchainImageViews(pContext->device,
                                        pContext->swapchainImageFormat,       
                                        pContext->swapchainImageCount,    // 创建交换链的
                                        pContext->swapchainImages);       // 图像视图
    if (!pContext->swapchainImageViews)
        return false;

    if (!create_main_thread_command_pool(pContext))    // 创建主线程命令池
        return false;

    if (!allocate_main_command_buffers(pContext))      // 分配主命令缓冲区
        return false;


    if (!create_main_render_pass(pContext))            // 创建主渲染通道
        return false;

    if(!create_swapchain_framebuffers(pContext))       // 为主渲染通道创建交换链帧缓冲区
        return false;

    if (!create_main_render_pass_pipeline_0(pContext,  // 为主渲染通道创建渲染管线 0
             "./triangle_vert.spv",
             "main",
             "./triangle_frag.spv",
             "main"))
        return false;


    if (!create_sync_objects(pContext))     // 创建同步用对象
        return false;

    log_info("渲染器上下文构建完毕.");

    return true;
}

static bool create_main_thread_command_pool(RendererContext* pContext)
{
    int queueFamilyIndex = -1;
    bool useSingleQueue = 
       has_queue_family_supports_both_graphics_and_presentation(pContext->physicalDevice,
           pContext->surface,
           &queueFamilyIndex);
    
    QueueFamilyIndices queueFamilyIndices = 
        find_queue_families(pContext->physicalDevice, pContext->surface);

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

    pContext->mainThreadCommandPool = createCommandPool(pContext->device, &createInfo);
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

    allocateCommandBuffers(pContext->device,
        &allocateInfo,
        pContext->mainCommandBuffers);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (pContext->mainCommandBuffers[i] == VK_NULL_HANDLE)
            return false;
    }

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

    pContext->mainRenderPass = createRenderPass(pContext->device, &createInfo);
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
            createFramebuffer(pContext->device, &createInfo);
        // 创建失败直接返回 false 退出函数
        if (pContext->swapchainFramebuffers[i] == VK_NULL_HANDLE)
            return false;
    }

    return true;
}

static bool create_main_render_pass_pipeline_0(
    RendererContext*    pContext,
    const char*         vertexSpvFilePath,
    const char*         vertexSpvEntryPoint,
    const char*         fragmentSpvFilePath,
    const char*         fragmentSpvEntryPoint
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
    // VkShaderModule（SPV 码）在创建图形管线时才会被编译链接成 GPU 机器码，且在管线创建完成后
    // 我们既可以调用 vkDestroyShaderModule 进行销毁也可以存着备后用（比如重建管线之类）
    pContext->mainRenderPassPipeline0.vertexShaderModule =         // 顶点着色器
        createShaderModule(pContext->device, vertexSpvFilePath);
    if (pContext->mainRenderPassPipeline0.vertexShaderModule == VK_NULL_HANDLE)
        return false;

    pContext->mainRenderPassPipeline0.fragmentShaderModule =       // 片元着色器
        createShaderModule(pContext->device, fragmentSpvFilePath);
    if (pContext->mainRenderPassPipeline0.fragmentShaderModule == VK_NULL_HANDLE)
        return false;

    // 1.5.（配置着色器阶段）填写 VkPipelineShaderStageCreateInfo
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};     // 顶点着色器
    vertexShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = pContext->mainRenderPassPipeline0.vertexShaderModule;
    vertexShaderStageInfo.pName  = vertexSpvEntryPoint;      
                                                // 指定 SPV 的函数入口点
                                                //（SPV 是可以由多个着色器文件编译得来的）

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};   // 片元着色器
    fragmentShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = pContext->mainRenderPassPipeline0.fragmentShaderModule;
    fragmentShaderStageInfo.pName  = fragmentSpvEntryPoint;

    // 创建 craeteInfo 数组备用
    VkPipelineShaderStageCreateInfo shaderStageInfos[] = {
        vertexShaderStageInfo,
        fragmentShaderStageInfo
    };


    // 2.（配置顶点输入阶段）填写 VkPipelineVertexInputStateCreateInfo
    // 顶点输入阶段需要设置两种结构体，分别是
    // VkVertexInputBindingDescription 和
    // VkVertexInputAttributeDescription

    VkVertexInputBindingDescription vertexInputBindingDescription = 
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
    vertexInputStateInfo.vertexBindingDescriptionCount   = 1; 
    vertexInputStateInfo.pVertexBindingDescriptions      =
        &vertexInputBindingDescription;
    vertexInputStateInfo.vertexAttributeDescriptionCount = vertexInputAttributeCount;
    vertexInputStateInfo.pVertexAttributeDescriptions    =
        vertexInputAttributeDescriptions;


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
    // 有关，现在我们不需要
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts    = NULL;

    // 8.5.创建管线布局
    pContext->mainRenderPassPipeline0.pipelineLayout = 
        createPipelineLayout(pContext->device, &pipelineLayoutInfo);
    if (pContext->mainRenderPassPipeline0.pipelineLayout == VK_NULL_HANDLE)
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
    createInfo.layout              = pContext->mainRenderPassPipeline0.pipelineLayout;
    createInfo.renderPass          = pContext->mainRenderPass;
    createInfo.subpass             = 0;


    // 9.创建图形管线
    pContext->mainRenderPassPipeline0.pipeline =
        createGraphicsPipeline(pContext->device, &createInfo);
    if (pContext->mainRenderPassPipeline0.pipeline == VK_NULL_HANDLE)
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
            createSemaphore("for swapchain image available synchronous",
                pContext->device,
                &semaphoreInfo);

        pContext->frameInFlightFences[i] = 
            createFence("for one frame in flight synchronous",
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
            createSemaphore("for render finished synchronous",
                pContext->device,
                &semaphoreInfo);
        
        if (pContext->renderFinishedSemaphores[i] == VK_NULL_HANDLE)
            return false;
    }

    return true;
}


void rctxDestroyRendererContext(RendererContext* pContext)
{
    log_info("销毁渲染器上下文...");

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

    /**** 同步对象相关 ****/

    uint32_t i = 0;
    for (; i < pContext->swapchainImageCount; i++)
    {
        if (pContext->renderFinishedSemaphores[i])          // 销毁渲染操作触发用信号量
            destroySemaphore(pContext->device, pContext->renderFinishedSemaphores[i]);
    }

    free(pContext->renderFinishedSemaphores);   // 释放数组堆内存

    for (i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (pContext->swapchainImageAvailableSemaphores[i]) // 销毁请求图像触发用信号量
            destroySemaphore(pContext->device,
                pContext->swapchainImageAvailableSemaphores[i]);

        if (pContext->frameInFlightFences[i])               // 销毁帧栅栏
            destroyFence(pContext->device, pContext->frameInFlightFences[i]);
    }

    /**** 管线对象相关 ****/

    if (pContext->mainRenderPassPipeline0.pipeline) // 销毁 mainRenderPassPipeline0 管线
        destroyPipeline(pContext->device, pContext->mainRenderPassPipeline0.pipeline);
    // 销毁 mainRenderPassPipeline0 管线布局
    if (pContext->mainRenderPassPipeline0.pipelineLayout)       
        destroyPipelineLayout(pContext->device,
            pContext->mainRenderPassPipeline0.pipelineLayout);
    // 销毁 mainRenderPassPipeline0 管线着色器模块
    if (pContext->mainRenderPassPipeline0.vertexShaderModule)
        destroyShaderModule(pContext->device,
            pContext->mainRenderPassPipeline0.vertexShaderModule);
    if (pContext->mainRenderPassPipeline0.fragmentShaderModule)
        destroyShaderModule(pContext->device,
            pContext->mainRenderPassPipeline0.fragmentShaderModule);

    /**** 命令池对象相关 ****/

    if (pContext->mainThreadCommandPool)                           // 销毁主线程命令池
        destroyCommandPool(pContext->device, pContext->mainThreadCommandPool);

    /*** 顶点缓冲区相关（临时） ***/    // TODO: 移到更加专门的地方

    if (pContext->triangle_vertexBuffer) // 先销毁缓冲区
        destroyBuffer(pContext->device, pContext->triangle_vertexBuffer);


    if (pContext->triangle_vertexBufferMemory) // 再销毁其内存
        freeDeviceMemory(pContext->device, pContext->triangle_vertexBufferMemory);

    /**** 交换链对象相关 ****/

    destroy_swapchain_related_resources(pContext);

    if (pContext->swapchain != VK_NULL_HANDLE)                     // 销毁交换链
        destroySwapchain(pContext->device, pContext->swapchain, NULL);
    
    /**** Vk 基础对象相关 ****/

    if (pContext->device != VK_NULL_HANDLE)                        // 销毁 Vk 设备
        destroyLogicalDevice(pContext->device);
    
    if (pContext->surface != VK_NULL_HANDLE)                       // 销毁窗口表面
        destroySurface(pContext->instance, pContext->surface);

    destroyInstance(pContext->instance);                           // 销毁 Vk 实例

    free(pContext);                                            // 释放渲染器上下文结构体
    pContext = NULL;                                           // 占用的内存

    log_info("销毁渲染器上下文完毕.");

    return;
}

/// @brief 该函数会销毁：交换链图像帧缓冲区、渲染通道、交换链图像视图和交换链图像数组.
static void destroy_swapchain_related_resources(RendererContext* pContext)
{
    if (pContext->swapchainFramebuffers)            // 销毁交换链帧缓冲区
    {
        for (int i = 0; i < pContext->swapchainImageCount; i++)   
        {
            if (pContext->swapchainFramebuffers[i])
                destroyFramebuffer(pContext->device,
                    pContext->swapchainFramebuffers[i]);
        }

        free(pContext->swapchainFramebuffers);      // 释放交换链帧缓冲区数组
        pContext->swapchainFramebuffers = NULL;     // 占用的堆内存
    }

    if (pContext->mainRenderPass)            // 销毁主渲染通道（因为描述了交换链图像做附件）
        destroyRenderPass(pContext->device, pContext->mainRenderPass);

    if (pContext->swapchainImageViews)       // 销毁交换链图像视图
        destroySwapchainImageViews(pContext->device,
            pContext->swapchainImageCount,
            &pContext->swapchainImageViews);

    if (pContext->swapchainImages)           // 释放交换链图像数组堆内存
    {
        free(pContext->swapchainImages);
        pContext->swapchainImages = NULL;
    }
}


// TODO: 想个办法让顶点缓冲区的创建变得通用？我的意思是——也许应该让 C# 端来决定创建怎样的
// 顶点缓冲区，传入 C 端 DrawFrame() 时附带某种 BindVertexBuffersInfo、DrawInfo
bool triangle_allocate_and_fill_vertex_buffer(
    RendererContext*    pContext,
    size_t              vertexDataSize,
    const VertexData*   pVertexData
)
{
    // 1.创建 VkBuffer
    VkBufferCreateInfo createInfo = {};
    createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size        = vertexDataSize;
    createInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    pContext->triangle_vertexBuffer = createBuffer("for triangle vertex buffer",
                                          pContext->device,
                                          &createInfo);
    if (pContext->triangle_vertexBuffer == VK_NULL_HANDLE)
        return false;

    // 2.获取 Buffer 的 Memory Requirements
    VkMemoryRequirements bufferMemoryRequirements = {};
    vkGetBufferMemoryRequirements(pContext->device,
        pContext->triangle_vertexBuffer,
        &bufferMemoryRequirements);

    // 3.获取物理设备内存属性
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(pContext->physicalDevice,
        &physicalDeviceMemoryProperties);

    // 4.根据 Buffer 的 Memory Requirements 填写 VkMemoryAllocateInfo
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex =
        get_device_memory_type_index(&physicalDeviceMemoryProperties,
            bufferMemoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 4.5.分配设备内存
    pContext->triangle_vertexBufferMemory =
        allocateDeviceMemory("for triangle vertex buffer",
            pContext->device,
            &memoryAllocateInfo);
    if (pContext->triangle_vertexBufferMemory == VK_NULL_HANDLE)
        return false;

    // 5.绑定 Buffer 至设备内存
    VkResult result = vkBindBufferMemory(pContext->device,
                          pContext->triangle_vertexBuffer,
                          pContext->triangle_vertexBufferMemory,
                          0);
    if (result != VK_SUCCESS)
    {
        log_error("Failed to bind a VkDeviceMemory with a VkBuffer! "
            "Error Code(VkResult): %d",
             result);

        return false;
    }

    // 6.填充 Buffer，这是通过将与缓冲区绑定的设备内存映射到 CPU 端的内存，然后将数据内存复制
    // 到被映射的内存中去来实现的
    void* pData;
    result = vkMapMemory(pContext->device,
                 pContext->triangle_vertexBufferMemory,
                 0,                  // 设备内存映射起始偏移量
                 vertexDataSize,     // 设备内存映射大小
                 0,
                 &pData);            // 映射到的 CPU 内存
    if (result != VK_SUCCESS)
    {
        log_error("Failed to map a VkDeviceMemory to host-visible memory! "
            "Error Code(VkResult): %d",
             result);

        return false;
    }

    memcpy(pData, pVertexData, vertexDataSize);

    vkUnmapMemory(pContext->device, pContext->triangle_vertexBufferMemory);

    return true;
}

static uint32_t get_device_memory_type_index(
    VkPhysicalDeviceMemoryProperties*   pPhysicalDeviceMemoryProperties,
    uint32_t                            requiredMemoryTypeBits,
    VkMemoryPropertyFlags               requiredMemoryPropertyFlags
)
{
    for (uint32_t i = 0; i < pPhysicalDeviceMemoryProperties->memoryTypeCount; i++)
    {
        if ((requiredMemoryTypeBits & (1 << i)) 
            && ((pPhysicalDeviceMemoryProperties->memoryTypes[i].propertyFlags
                   & requiredMemoryPropertyFlags) == requiredMemoryPropertyFlags))
            return i;
    }

    log_error("%s()：要求的内存类型和属性在物理设备中不可用！要求的内存类型 Bits：%x；"
        "要求的内存属性 Flags：%x",
        requiredMemoryTypeBits, requiredMemoryPropertyFlags);

    return UINT32_MAX;
}


void rctxDrawFrame(                       // TODO: rctxDrawFrame()
    RendererContext*                    pContext,
    bool                                isFramebufferResized,
    MainRenderPassPipeline0DrawInfo*    pMainRenderPassPipeline0DrawInfo // TODO: 设计某种 DrawInfo，让调用者传入必要信息
)
{
    // 检查传入参数
    if (!pMainRenderPassPipeline0DrawInfo)
    {
        log_error("%s()：传入了无效的 pMainRenderPassPipeline0DrawInfo，"
            "该参数不得为 NULL！");

        return;
    }

    static int frameInFlightIndex = 0;

    // 0.等待 In Flight 帧栅栏
    vkWaitForFences(pContext->device,
        1,
        &pContext->frameInFlightFences[frameInFlightIndex],
        VK_TRUE,
        UINT64_MAX);

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
    //（确保下文我们一定会使用该帧栅栏如调用 vkQueueSubmit() 时，才重置，不然的话如果在检查交换
    // 链是否需要重建之前就重置帧栅栏，当遇到交换链需要重建时 draw_frame 会直接 return，
    // 这样在该次 draw_frame 被重置的栅栏就没有任何提交函数会使用和触发它，而下一次等待它的
    // draw_frame 就会永远等待它，造成死锁）
    vkResetFences(pContext->device,
        1,
        &pContext->frameInFlightFences[frameInFlightIndex]);
    // 现在可以安全处理该帧的命令缓冲区了

    // 2.重置主命令缓冲区
    vkResetCommandBuffer(pContext->mainCommandBuffers[frameInFlightIndex], 0);

    // 3.录制主命令缓冲区，传入对应交换链图像索引（也对应帧缓冲区索引）
    record_main_command_buffer(pContext,        // TODO: record_main_command_buffer()
        frameInFlightIndex,
        swapchainImageIndex,
        pMainRenderPassPipeline0DrawInfo);
    
    // 4.准备提交主命令缓冲区
    
    // 指定提交要等待的所有信号量（这里只有一个信号量需要等待，即 交换链图像可用 信号量）
    VkSemaphore semaphoresToWait[] = {
        pContext->swapchainImageAvailableSemaphores[frameInFlightIndex]
    };

    // 指定所有等待信号量对应的等待阶段
    //（这里我们直到将要输出到颜色附件阶段时才会用到交换链图像，所以在此阶段
    // 等待 交换链图像可用 信号量）
    //（注：以上说法是错误的，实际上在渲染通道开始时，由于没有定义外部子通道依赖，隐式
    // 的外部子通道依赖会使得引用图像的子通道（这里是引用了交换链图像）会在任何可能的时间点对
    // 这些图像进行布局转换（Desc.initLayout 到 Ref.layout），所以从这个方面看，
    // 这里的 waitOnStages 应该是 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT 阶段才对，但这样设置
    // 会导致不必要的空闲等待，毕竟在这里我们子通道前期阶段的执行确实不会用到交换链图像；
    // 所以更好的方法是显式定义与外部子通道的子通道依赖，
    // 告诉 Vulkan 我们该子通道的 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 阶段（真正
    // 开始使用交换链图像的阶段）依赖于 VK_SUBPASS_EXTERNAL（特殊值，指代隐式的外部子通道）的 
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 阶段 —— 这个阶段的完成意味着请求的
    // 交换链图像可用）
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
    queueSubmit(pContext->graphicsQueue,
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
    // 所以这里提交了呈现，进入下一个循环时，假设上个循环的呈现操作不够快，
    // 还没来得及 “消耗” 指定的 presentInfo.pWaitSemaphores 中的信号量时，
    // 这些信号量在下个循环，甚至是下个循环执行到提交渲染操作（调用 vkQueueSubmit）、GPU 执行
    // 渲染操作完毕，将要触发指定要触发的信号量（也就是呈现要等待的信号量）时，
    // 它们都还未被上个循环的呈现操作消耗掉（仍处于已触发状态），这便导致了验证层
    // VUID-vkQueueSubmit-pSignalSemaphores-00067 报错.
    // 简单来说，vkQueuePresentKHR() 不像 vkQueueSubmit()，前者并没有提供触发信号量或栅栏之类
    // 的同步原语的方法 (在没有扩展的情况下)，所以重用呈现操作所消耗的信号量对象的时机并不太清晰，
    // 详见 https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html.
    // 推荐的解决方案是，基于交换链图像的数量创建多个信号量，每次 InFlight 循环所用的交换链图像
    // 其渲染和呈现操作均使用 swapchainImageIndex 来取信号量使用，一个交换链图像对应一个信号量，
    // 这样上个循环使用的信号量就不关下个循环的事，等到下一次使用同一个信号量时，消耗它的呈现操作
    // 必然是早就完成了的.
    result = vkQueuePresentKHR(pContext->presentationQueue, &presentInfo);
    // 如果交换链过时或为次优，重建交换链
    if (result == VK_ERROR_OUT_OF_DATE_KHR 
        || result == VK_SUBOPTIMAL_KHR
        || isFramebufferResized)
        recreate_swapchain(pContext);
    else if (result != VK_SUCCESS)
        log_error("%s()：交换链图像的呈现发生错误！Error Code(VkResult)：%d",
            __func__, result);

    frameInFlightIndex = (frameInFlightIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

static bool record_main_command_buffer(     // TODO: record_main_command_buffer()
    RendererContext*                    pContext,
    uint32_t                            frameInFlightIndex,
    uint32_t                            swapchainFramebufferIndex,
//  TODO: 设计某种 DrawInfo，让调用者传入必要信息，例如：
    MainRenderPassPipeline0DrawInfo*    pMainRenderPassPipeline0DrawInfo
//  MainRenderPassPipeline1DrawInfo*    pMainRenderPassPipeline1DrawInfo;
//  DebugRenderPassPipeline0DrawInfo*   pDebugRenderPassPipeline0DrawInfo;
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

    if (!beginCommandBuffer(label, commandBuffer, &commandBufferBeginInfo))
        return false;

    // TODO: 根据传入的已分类的顶点缓冲区，按类别依次录制渲染（子）通道（绑定、绘制），一次录制包含整一帧的所有工作

    // 2.开始录制渲染通道
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

    // 注：调用 vkCmdBeginRenderPass() 会隐式开始第 0 个 subpass
    vkCmdBeginRenderPass(commandBuffer,
        &renderPassBeginInfo,
        VK_SUBPASS_CONTENTS_INLINE);

        // 2.1.为第 0 个 subpass 绑定渲染管线
        vkCmdBindPipeline(commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pContext->mainRenderPassPipeline0.pipeline);

        // 2.2.处理绑定管线的动态状态
        VkViewport viewport = {};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = pContext->swapchainExtent.width;
        viewport.height     = pContext->swapchainExtent.height;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset   = (VkOffset2D){0, 0};
        scissor.extent   = pContext->swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // 2.3.为绑定管线绑定顶点缓冲区
        
        // TODO: 顶点缓冲区不应该在 pContext 里，应该传进来
        VkBuffer mainRenderPassPipeline0VertexBuffers [] = {
            pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.vertexBuffer
        };
        
        VkDeviceSize mainRenderPassPipeline0VertexBufferOffsets[] = {
            pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.offset
        };
        
        vkCmdBindVertexBuffers(commandBuffer,
            0,
            1,
            mainRenderPassPipeline0VertexBuffers,
            mainRenderPassPipeline0VertexBufferOffsets);

        // VkBuffer vertexBuffers[]             = {pContext->triangle_vertexBuffer};
        // VkDeviceSize vertexBufferOffsets[]   = {0};
        // vkCmdBindVertexBuffers(commandBuffer,
        //     0,                    // 起始绑定索引，管线规定的，不得乱写
        //     1,                    // 绑定数 (起始往下)，不得乱写
        //     vertexBuffers,        // Buffer 数组，个数对应绑定数，也不得乱写
        //     vertexBufferOffsets); // Buffer 数组的 offset 数组，个数对应绑定数，也不得乱写

        // 2.4.Draw Call

        for (int i = 0;
             i < pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.drawItemCount;
             i++)
        {
            vkCmdDraw(commandBuffer,
                pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.pDrawItemInfos[i]
                    .vertexCount,
                pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.pDrawItemInfos[i]
                    .instanceCount,
                pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.pDrawItemInfos[i]
                    .firstVertex,
                pMainRenderPassPipeline0DrawInfo->vertexBufferInfo.pDrawItemInfos[i]
                    .firstInstance);
        }

        // vkCmdDraw(commandBuffer, vertexDrawCount, 1, 0, 0);

    // 对于目前仅绘制三角形的 mainRenderPass，我们只有一个 subpass，
    // 所以现在可以结束 RenderPass 了
    //（对于多个 subpass 的 RenderPass，我们要按对应 RenderPass 的设计，
    // 调用 vkCmdNextSubpass() 以表示进入下一个 subpass）

    // 3.结束录制渲染通道
    vkCmdEndRenderPass(commandBuffer);
    
    // 我们目前只有一个 mainRenderPass，所以现在可以结束录制命令缓冲区了
    //（对于多个 RenderPass，我们调用 vkCmdBeginRenderPass() 以录制下一个 RenderPass）

    // 4.结束录制命令缓冲区
    if (!endCommandBuffer(label, commandBuffer))
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

    log_info("触发了交换链重建，oldSwapchain: %p, extent: %d x %d",
        pContext->swapchain,
        pContext->swapchainExtent.width,
        pContext->swapchainExtent.height);

    vkDeviceWaitIdle(pContext->device);

    // 在重新创建交换链和相关对象前，先销毁旧交换链相关的对象
    destroy_swapchain_related_resources(pContext);

    // 引用为旧交换链
    VkSwapchainKHR oldSwapchain = pContext->swapchain;

    // 重新创建交换链
    pContext->swapchain = createSwapchain(pContext->window,
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
        destroySwapchain(pContext->device, oldSwapchain, NULL);

    // 重新创建交换链图像视图
    pContext->swapchainImageViews = createSwapchainImageViews(pContext->device,
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

