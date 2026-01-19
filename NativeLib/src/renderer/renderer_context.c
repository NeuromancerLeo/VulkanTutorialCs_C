#include "renderer_context.h"

static bool triangle_create_render_pass(RendererContext* pContext);
static bool trigangle_create_swapchain_framebuffers(RendererContext* pContext);
static bool triangle_create_graphics_pipeline(
    RendererContext*    pContext,

    const char*         vertexSpvFilePath,
    const char*         vertexSpvEntryPoint,

    const char*         fragmentSpvFilePath,
    const char*         fragmentSpvEntryPoint
);


RendererContext* new_renderer_context()
{
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif

    // 分配堆内存
    RendererContext* pContext = (RendererContext*)calloc(1, sizeof(RendererContext));
    if (!pContext) return NULL;

    return pContext;
}


bool create_renderer_context(RendererContext* pContext, GLFWwindow* window)
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


    if (!triangle_create_render_pass(pContext))             // 为三角形绘制创建渲染通道
        return false;

    if(!trigangle_create_swapchain_framebuffers(pContext))  // 为三角形绘制创建帧缓冲区
        return false;

    if (!triangle_create_graphics_pipeline(pContext,        // 为三角形绘制创建渲染管线
             "./triangle_vert.spv",
             "main",
             "./triangle_frag.spv",
             "main"))
        return false;

    log_info("渲染器上下文构建完毕.");

    return true;
}

// 以下是应用层函数，这些函数用来为 绘制三角形 创建对应的管线资源.
// （说方法属于应用层的意思是这些资源是高度绑定“绘制三角形”这个需求的，所以这个函数
// 不会写到 vulkan_wrapper（负责提供基础函数）中去，renderer_context（负责调用基础函数）是
// 负责应用类函数的好地方

static bool triangle_create_render_pass(RendererContext* pContext)
{
    // 1.填写 VkAttachmentDescription 来描述附件（使用多少个附件就要填多少个）
    // 对于渲染一帧三角形，我们只有一个附件，即（作颜色附件的）交换链中的图像
    // 所以这里只需要填一个该结构体来描述这个附件即可
    VkAttachmentDescription colorAttachment01Description = {};
    // 该附件格式对应为交换链图像的格式
    colorAttachment01Description.format  = pContext->swapchainImageFormat;
    colorAttachment01Description.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment01Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment01Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment01Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment01Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // 因为加载时都会清除，所以附件的初始布局可直接描述为未定义（是期望的）
    // 我们希望渲染完后将这个附件进行窗口呈现，故最终布局描述为呈现源（是会自动转换的）
    colorAttachment01Description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment01Description.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 2.设置子通道
    // 渲染一帧三角形，我们只描述一个子通道即可
    // 首先填写 VkAttachmentReference 来描述子通道会引用的附件
    // attachment 指定针对 VkAttachmentDescription 的索引值
    // layout 指定子流程开始时附件需要转化到的布局
    VkAttachmentReference colorAttachment01Reference = {};
    colorAttachment01Reference.attachment = 0;
    colorAttachment01Reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // 2.5.填写 VkSubpassDescription 来描述子通道
    VkSubpassDescription subpass01Description = {};
    // 该子通道指定为图形用
    subpass01Description.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // 绑定颜色附件数组
    // 注：该数组索引值直接对应片段着色器中的代码： 
    // layout(location = 索引值) out vec4 outColorxxx; （用于输出到颜色附件）
    subpass01Description.colorAttachmentCount = 1;
    subpass01Description.pColorAttachments    = &colorAttachment01Reference;

    // 3.设置子通道依赖
    // 通过填写 VkSubpassDependency 来描述 
    // 这里只有一个子通道，所以略过这一步

    // 4.创建渲染通道
    // 填写 VkRenderPassCreateInfo
    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;    // 引用 VkAttachmentDescription 数组
    createInfo.pAttachments    = &colorAttachment01Description;
    createInfo.subpassCount    = 1;    // 引用 VkSubpassDescription 数组
    createInfo.pSubpasses      = &subpass01Description;
    createInfo.dependencyCount = 0;    // 应用 VkSubpassDependency 数组
    createInfo.pDependencies   = NULL;

    pContext->triangle_renderPass = createRenderPass(pContext->device, &createInfo);
    if (pContext->triangle_renderPass == VK_NULL_HANDLE)
        return false;

    return true;
}

static bool trigangle_create_swapchain_framebuffers(RendererContext* pContext)
{
    // 为帧缓冲区数组分配堆内存
    pContext->triangle_swapchainFramebuffers = // 这里需要一个交换链图像对应一个帧缓冲区
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
        createInfo.renderPass      = pContext->triangle_renderPass;
        // 附件数组信息
        createInfo.attachmentCount = 1;
        createInfo.pAttachments    = attachments;
        // 附件的尺寸（帧缓冲区规定其所有附件都需要为相同尺寸）
        createInfo.width           = pContext->swapchainExtent.width;
        createInfo.height          = pContext->swapchainExtent.height;
        // 需要使用的附件层数（我们的附件（交换链图像）只有一层，所以这里填 1）
        createInfo.layers          = 1;
        
        pContext->triangle_swapchainFramebuffers[i] =
            createFramebuffer(pContext->device, &createInfo);
        // 创建失败直接返回 false 退出函数
        if (pContext->triangle_swapchainFramebuffers[i] == VK_NULL_HANDLE)
            return false;
    }

    return true;
}

static bool triangle_create_graphics_pipeline(
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
    pContext->triangle_vertexShaderModule =         // 顶点着色器
        createShaderModule(pContext->device, vertexSpvFilePath);
    if (pContext->triangle_vertexShaderModule == VK_NULL_HANDLE)
        return false;

    pContext->triangle_fragmentShaderModule =       // 片元着色器
        createShaderModule(pContext->device, fragmentSpvFilePath);
    if (pContext->triangle_fragmentShaderModule == VK_NULL_HANDLE)
        return false;

    // 1.5.（配置着色器阶段）填写 VkPipelineShaderStageCreateInfo
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};     // 顶点着色器
    vertexShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = pContext->triangle_vertexShaderModule;
    vertexShaderStageInfo.pName  = vertexSpvEntryPoint;      
                                                // 指定 SPV 的函数入口点
                                                //（SPV 是可以由多个着色器文件编译得来的）

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};   // 片元着色器
    fragmentShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = pContext->triangle_fragmentShaderModule;
    fragmentShaderStageInfo.pName  = fragmentSpvEntryPoint;

    // 创建 craeteInfo 数组备用
    VkPipelineShaderStageCreateInfo shaderStageInfos[] = {
        vertexShaderStageInfo,
        fragmentShaderStageInfo
    };


    // 2.（配置顶点输入阶段）填写 VkPipelineVertexInputStateCreateInfo
    // 对于绘制三角形，其顶点数据是在着色器里直接硬编码的, 所以这里不需要为加载顶点数据
    // 填写任何描述结构体
    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
    vertexInputStateInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateInfo.vertexBindingDescriptionCount   = 0; 
    vertexInputStateInfo.pVertexBindingDescriptions      = NULL;
    vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputStateInfo.pVertexAttributeDescriptions    = NULL;

    // 2.5（配置顶点输入阶段）填写 VkPipelineInputAssemblyStateCreateInfo
    VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyInfo = {};
    vertexInputAssemblyInfo.sType = 
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vertexInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


    // 3.（配置光栅化阶段）填写 VkPipelineRasterizationStateCreateInfo
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


    // 4.（配置多重采样阶段，MSAA）填写 VkPipelineMultisampleStateCreateInfo
    // 禁用
    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo = {};
    multisamplingStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo.sampleShadingEnable  = VK_FALSE;
    multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


    // 5.（配置深度和模板测试阶段）填写 VkPipelineDepthStencilStateCreateInfo
    // 不需要


    // 6.（配置颜色混合阶段）填写 VkPipelineColorBlendAttachmentState，
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

    // 6.5 填写 VkPipelineColorBlendStateCreateInfo, 其包含颜色混合的全局设置（覆盖性的）
    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
    colorBlendStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // 是否启用全局覆盖性颜色混合设置，启用会使得上面所有颜色附件的颜色混合设置都被忽略，
    // 全部强制使用逻辑操作计算混合
    colorBlendStateInfo.logicOpEnable   = VK_FALSE;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments    = &colorBlendAttachmentState;


    // 7.（配置 Pipeline Layout）填写 VkPipelineLayoutCreateInfo
    // Pipeline Layout 设置的是 VkDescriptorSetLayout 数组信息和推送常量，与着色器 uniform
    // 有关，现在我们不需要
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts    = NULL;

    // 7.5.创建管线布局
    pContext->triangle_pipelineLayout = 
        createPipelineLayout(pContext->device, &pipelineLayoutInfo);
    if (pContext->triangle_pipelineLayout == VK_NULL_HANDLE)
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
    createInfo.layout              = pContext->triangle_pipelineLayout;
    createInfo.renderPass          = pContext->triangle_renderPass;
    createInfo.subpass             = 0;


    // 8.创建图形管线
    pContext->triangle_pipeline = createGraphicsPipeline(pContext->device, &createInfo);
    if (pContext->triangle_pipeline == VK_NULL_HANDLE)
        return false;

    return true;
}


void destroy_renderer_context(RendererContext* pContext)
{
    log_info("销毁渲染器上下文...");

    if (!pContext)
    {
        log_info("%s : 给定渲染器上下文地址无效, 退出.", __func__);
        return;
    }

    if (pContext->instance == VK_NULL_HANDLE)
    {
        log_info("%s : 给定渲染器上下文不含有效的 VkInstance 句柄，不会销毁任何内容，退出.",
            __func__);
        return;
    }

    // TODO: vkDeviceWaitIdle

    if (pContext->triangle_pipeline)                          // 销毁三角形管线
        destroyPipeline(pContext->device, pContext->triangle_pipeline);

    if (pContext->triangle_pipelineLayout)                    // 销毁三角形管线布局
        destroyPipelineLayout(pContext->device, pContext->triangle_pipelineLayout);

    if (pContext->triangle_vertexShaderModule)                // 销毁三角形管线着色器模块
        destroyShaderModule(pContext->device, pContext->triangle_vertexShaderModule);
    if (pContext->triangle_fragmentShaderModule)
        destroyShaderModule(pContext->device, pContext->triangle_fragmentShaderModule);

    if (pContext->triangle_swapchainFramebuffers)             // 销毁三角形绘制用帧缓冲区
    {
        for (int i = 0; i < pContext->swapchainImageCount; i++)   
        {
            if (pContext->triangle_swapchainFramebuffers[i])
                destroyFramebuffer(pContext->device,
                    pContext->triangle_swapchainFramebuffers[i]);
        }

        free(pContext->triangle_swapchainFramebuffers);       // 释放帧缓冲区数组
        pContext->triangle_swapchainFramebuffers = NULL;      // 占用的堆内存
    }

    if (pContext->triangle_renderPass)                        // 销毁三角形绘制用渲染通道
        destroyRenderPass(pContext->device, pContext->triangle_renderPass);


    if (pContext->swapchainImageViews)                             // 销毁交换链图像视图
        destroySwapchainImageViews(pContext->device,
            pContext->swapchainImageCount,
            &pContext->swapchainImageViews);

    if (pContext->swapchain != VK_NULL_HANDLE)                     // 销毁交换链及其图像
        destroySwapchain(pContext->device,
            pContext->swapchain,
            &pContext->swapchainImages);

    if (pContext->device != VK_NULL_HANDLE)                        // 销毁 Vk 设备
        destroyLogicalDevice(pContext->device);
    
    if (pContext->surface != VK_NULL_HANDLE)                       // 销毁窗口表面
        destroySurface(pContext->instance, pContext->surface);

    destroyInstance(pContext->instance);                           // 销毁 Vk 实例

    free(pContext);                                                // 释放渲染上下文结构体
    pContext = NULL;                                               // 占用的内存

    log_info("销毁渲染器上下文完毕.");

    return;
}


