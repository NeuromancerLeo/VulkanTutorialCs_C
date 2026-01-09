#include "render_context.h"


RenderContext* new_render_context()
{
#ifndef DEBUG
    log_set_level(LOG_INFO);
#endif

    // 分配堆内存
    RenderContext* pContext = (RenderContext*)calloc(1, sizeof(RenderContext));
    if (!pContext) return NULL;

    return pContext;
}


bool create_render_context(RenderContext* pContext, GLFWwindow* window)
{
    log_info("开始构建渲染上下文...");

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
                                        pContext->swapchainImages);       // 图形视图
    if (!pContext->swapchainImageViews)
        return false;

    log_info("渲染上下文构建完毕.");

    return true;
}


void destroy_render_context(RenderContext* pContext)
{
    log_info("销毁渲染上下文...");

    if (!pContext)
    {
        log_info("%s : 给定渲染上下文地址无效, 退出.", __func__);
        return;
    }

    if (pContext->instance == VK_NULL_HANDLE)
    {
        log_info("%s : 给定渲染上下文不含有效的 VkInstance 句柄，不会销毁任何内容，退出.",
            __func__);
        return;
    }

    // TODO: vkDeviceWaitIdle

    if (pContext->swapchainImageViews)                             // 销毁交换链图像视图
        destroySwapchainImageViews(pContext->device,               // 并释放其数组占用的
            pContext->swapchainImageCount,                         // 内存
            &pContext->swapchainImageViews);

    if (pContext->swapchain != VK_NULL_HANDLE)                     // 销毁交换链
        destroySwapchain(pContext->device, pContext->swapchain);
    
    if (pContext->swapchainImages)                                 // 释放交换链图像数组
    {                                                              // 占用的内存
        free(pContext->swapchainImages);
        pContext->swapchainImages = NULL;
    }

    if (pContext->device != VK_NULL_HANDLE)                        // 销毁 Vk 设备
        destroyLogicalDevice(pContext->device);
    
    if (pContext->surface != VK_NULL_HANDLE)                       // 销毁窗口表面
        destroySurface(pContext->instance, pContext->surface);

    destroyInstance(pContext->instance);                           // 销毁 Vk 实例

    free(pContext);                                                // 释放渲染上下文结构体
    pContext = NULL;                                               // 占用的内存

    log_info("销毁渲染上下文完毕.");

    return;
}


// static VkPipeline create_graphics_pipeline_for_HelloTriangle(
//     RenderContext*      pContext,

//     uint32_t            vertexSpvWordSize,
//     const uint32_t*     vertexSpvWords,
//     const char*         vertexSpvEntryPoint,

//     uint32_t            fragmentSpvWordSize,
//     const uint32_t*     fragmentSpvWords,
//     const char*         fragmentSpvEntryPoint
// )
// {
//     // 0.指定使用的动态状态（即不需要静态烘培到管线的状态）
//     VkDynamicState dynamicStates[] = {
//         VK_DYNAMIC_STATE_VIEWPORT,      // 视口大小
//         VK_DYNAMIC_STATE_SCISSOR        // 剪裁矩形
//     };

//     VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
//     dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//     dynamicStateInfo.dynamicStateCount = 2;
//     dynamicStateInfo.pDynamicStates    = dynamicStates;

//     // 对于视口和剪裁矩形，由于我们指定它们为动态的状态，故在这只需要指定它们的数量
//     // 而不指定具体结构体指针，实际的视口和剪裁矩形将会要在绘制时设置
//     VkPipelineViewportStateCreateInfo viewportStateInfo = {};
//     viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//     viewportStateInfo.viewportCount = 1;
//     viewportStateInfo.scissorCount  = 1;


//     // 1.（配置着色器阶段）创建 VkShaderModule
//     // VkShaderModule（SPV 码）在创建图形管线时才会被编译链接成 GPU 机器码，且在管线创建完成后
//     // 我们可以立即调用 vkDestroyShaderModule 进行销毁
//     VkShaderModule vertexShaderModule =         // 顶点着色器
//         createShaderModule(pContext->device, vertexSpvWordSize, vertexSpvWords);

//     VkShaderModule fragmentShaderModule =       // 片元着色器
//         createShaderModule(pContext->device, fragmentSpvWordSize, fragmentSpvWords);

//     // 1.5.（配置着色器阶段）填写 VkPipelineShaderStageCreateInfo
//     VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};     // 顶点着色器
//     vertexShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     vertexShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
//     vertexShaderStageInfo.module = vertexShaderModule;
//     vertexShaderStageInfo.pName  = vertexSpvEntryPoint;      
//                                                 // 指定 SPV 的函数入口点
//                                                 //（SPV 是可以由多个着色器文件编译得来的）

//     VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};   // 片元着色器
//     fragmentShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     fragmentShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
//     fragmentShaderStageInfo.module = fragmentShaderModule;
//     fragmentShaderStageInfo.pName  = fragmentSpvEntryPoint;

//     // 创建 craeteInfo 数组备用
//     VkPipelineShaderStageCreateInfo shaderStageInfos[] = {
//         vertexShaderStageInfo,
//         fragmentShaderStageInfo
//     };


//     // 2.（配置顶点输入阶段）填写 VkPipelineVertexInputStateCreateInfo
//     // 教程目前为止在着色器里直接硬编码顶点数据, 所以该结构体暂时表示为不需要加载顶点数据
//     VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
//     vertexInputStateInfo.sType = 
//         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//     vertexInputStateInfo.vertexBindingDescriptionCount   = 0; 
//     vertexInputStateInfo.pVertexBindingDescriptions      = NULL;
//     vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
//     vertexInputStateInfo.pVertexAttributeDescriptions    = NULL;

//     // 2.5（配置顶点输入阶段）填写 VkPipelineInputAssemblyStateCreateInfo
//     VkPipelineInputAssemblyStateCreateInfo vertexInputAssemblyInfo = {};
//     vertexInputAssemblyInfo.sType = 
//         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//     vertexInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//     vertexInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;


//     // 3.（配置光栅化阶段）填写 VkPipelineRasterizationStateCreateInfo
//     VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
//     rasterizationStateInfo.sType = 
//         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//     rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
//     rasterizationStateInfo.depthClampEnable        = VK_FALSE;
//     rasterizationStateInfo.depthBiasEnable         = VK_FALSE;
//     rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
//     rasterizationStateInfo.lineWidth   = 1.0f;
//     // 定义顺时针顶点顺序为正面，逆时针顶点顺序为背面
//     rasterizationStateInfo.frontFace   = VK_FRONT_FACE_CLOCKWISE; 
//     rasterizationStateInfo.cullMode    = VK_CULL_MODE_BACK_BIT;   


//     // 4.（配置多重采样阶段）填写 VkPipelineMultisampleStateCreateInfo
//     // 暂时禁用
//     VkPipelineMultisampleStateCreateInfo multisamplingStateInfo = {};
//     multisamplingStateInfo.sType =
//         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//     multisamplingStateInfo.sampleShadingEnable  = VK_FALSE;
//     multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


//     // 5.（配置深度和模板测试阶段）填写 VkPipelineDepthStencilStateCreateInfo
//     // 暂不需要

//     // 6.（配置颜色混合阶段）
//     // 6.1 填写 VkPipelineColorBlendAttachmentState，其包含针对帧缓冲区每个附件的配置
//     // 暂不启用，仅一个
//     VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
//     colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
//                                           | VK_COLOR_COMPONENT_R_BIT
//                                           | VK_COLOR_COMPONENT_G_BIT
//                                           | VK_COLOR_COMPONENT_B_BIT;
//     colorBlendAttachmentState.blendEnable    = VK_FALSE;

//     // 6.2 填写 VkPipelineColorBlendStateCreateInfo, 其包含颜色混合的全局设置
//     VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
//     colorBlendStateInfo.sType =
//         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//     colorBlendStateInfo.logicOpEnable   = VK_FALSE;
//     colorBlendStateInfo.attachmentCount = 1;
//     colorBlendStateInfo.pAttachments    = &colorBlendAttachmentState;



// }