#include "vulkan_wrapper.h"

#ifdef DEBUG
    static const bool enableValidationLayers = true;
#else
    static const bool enableValidationLayers = false;
#endif

static const char* requiredValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const char* requiredDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static bool check_instance_layer_properties(void);
static void check_instance_extension_properties(void);
static bool check_device_extension_properties(VkPhysicalDevice physicalDevice);
static bool is_physical_device_suitable(
    VkPhysicalDevice    physicalDevice, 
    VkSurfaceKHR        surface
);
static void dump_physical_device_properties(VkPhysicalDevice physicalDevice);


VkInstance createInstance(void)
{
    // 0.检查验证层是否开启并可用
    if (enableValidationLayers && !check_instance_layer_properties())
    {
        fprintf(stderr, "Validation layers requested, but not available!\n");

        return VK_NULL_HANDLE;
    }

    // 1.指定 ApplicationInfo
    VkApplicationInfo appInfo = {};
    appInfo.sType                       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName            = "Hello Triangle";
    appInfo.applicationVersion          = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName                 = "No Engine";
    appInfo.engineVersion               = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion                  = VK_API_VERSION_1_3;

    // 1.5.查询所有可用扩展
    check_instance_extension_properties();

    // 2.获取 GLFW 所需扩展的名称标识
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // 打印
    {
        fprintf(stdout, "GLFW required instance extensions:\n");
        for (int i = 0; i < glfwExtensionCount; i++)
        {
            fprintf(stdout,
                ESC_FCOLOR_BRIGHT_BLUE "    %s\n" ESC_RESET,
                glfwExtensions[i]);
        }
    }

    // 3.指定 InstanceCreateInfo
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo         = &appInfo;
    createInfo.enabledExtensionCount    = glfwExtensionCount;   // 这两行启用 GLFW 
    createInfo.ppEnabledExtensionNames  = glfwExtensions;       // 的所需扩展
    createInfo.enabledLayerCount        = 0;
    if (enableValidationLayers)     // 若启用验证层
    { 
        uint32_t requiredValidationLayerCount = 
            sizeof(requiredValidationLayers) / sizeof(requiredValidationLayers[0]);

        createInfo.enabledLayerCount    = requiredValidationLayerCount;
        createInfo.ppEnabledLayerNames  = requiredValidationLayers;
    }

    // 4.创建 Vulkan 实例
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, 
            "Failed to create a VkInstance! Error Code(VkResult): %d\n", result);
        
        return VK_NULL_HANDLE;
    }

    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "成功创建了一个 VkInstance！\n",
        __DATE__, __TIME__);

    return instance;
}

/// @brief 查询对 VkInstance 可用的层并打印出来，并检查请求的层是否可用.
///
/// @return 当检查到有请求的层不可用时，该函数会打印相关信息，并返回 `false`
static bool check_instance_layer_properties(void)
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    fprintf(stdout,
        "%s: Found" ESC_FCOLOR_BRIGHT_GREEN " %u " ESC_RESET
        "available VkInstance layers:\n",
        __func__, layerCount);
    
    if (layerCount < 1)
    {
        fprintf(stderr, 
            ESC_FCOLOR_RED
            "    No avaliable VkInstance layers could be found!\n" ESC_RESET);
        return false;
    }
        
    VkLayerProperties layers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, layers);
    
    // 打印全部可用层名
    for (int i = 0; i < layerCount; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BRIGHT_GREEN "    %s\n" ESC_RESET,
            layers[i].layerName);
    }
    // 打印我们请求的层名
    fprintf(stdout, "Application required validation layers:\n");
    
    uint32_t requiredValidationLayerCount = 
        sizeof(requiredValidationLayers) / sizeof(requiredValidationLayers[0]);
    for (int i = 0; i < requiredValidationLayerCount ; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BLUE "    %s\n" ESC_RESET, requiredValidationLayers[i]);
    }

    // 检查 validationLayer 中的层是否可用
    for (int i = 0; i < requiredValidationLayerCount; i++)
    {
        bool layerFound = false;
        for (int j = 0; j < layerCount; j++)
        {
            if (strcmp(requiredValidationLayers[i], layers[j].layerName))
                continue;

            layerFound = true;
            break;
        }
        
        // 只要有一个找不到就返回 false
        if(!layerFound)
        {
            fprintf(stderr,
                "Not supported layer: %s, can't found in available layers!",
                requiredValidationLayers[i]);
            return false;
        }
    }

    return true;
}

/// @brief 查询对 Vulkan Instance 可用的扩展并打印出来.
static void check_instance_extension_properties(void)
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    fprintf(stdout,
        "%s: Found" ESC_FCOLOR_BRIGHT_GREEN " %u " ESC_RESET
        "available VkInstance extensions:\n",
        __func__, extensionCount);

    if (extensionCount < 1)
    {
        fprintf(stderr, 
            ESC_FCOLOR_RED
            "    No avaliable VkInstance extensions could be found!\n" ESC_RESET);
        return;
    }
    
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    for (int i = 0; i < extensionCount; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BRIGHT_GREEN "    %s\n" ESC_RESET,
            extensions[i].extensionName);
    }
    
}


void destroyInstance(VkInstance instance)
{   
    vkDestroyInstance(instance, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroyInstance！\n" ESC_RESET,
        __DATE__, __TIME__);
}


VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
            "Faild to create a VkSurfaceKHR! Error Code(VkResult): %d\n",
            result);

        return VK_NULL_HANDLE;
    }

    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "成功创建了一个 VkSurfaceKHR！\n",
        __DATE__, __TIME__);

    return surface;
}


void destroySurface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroySurfaceKHR！\n" ESC_RESET,
        __DATE__, __TIME__);
}


VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    // 1.查询可用的物理设备
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0)
    {
        fprintf(stderr, "Failed to find GPUs with Vulkan support!\n");
        
        return VK_NULL_HANDLE;
    }

    // 2.为查询到的物理设备分配数组
    VkPhysicalDevice physicalDevices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

    // 3.尝试选择可用的显卡
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    for (int i = 0; i < deviceCount; i++)
    {
        if (is_physical_device_suitable(physicalDevices[i], surface))
        {
            physicalDevice = physicalDevices[i];
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        fprintf(stderr, "Failed to find a suitable GPU!\n");

        return VK_NULL_HANDLE;
    }
        
    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "成功选取了一个物理设备！\n",
        __DATE__, __TIME__);

    dump_physical_device_properties(physicalDevice);

    return physicalDevice;
}

/// @brief 该函数用于检查传入的物理设备的某个属性/支持功能是否符合要求.
///
/// （至于具体要求详见函数）
///
/// @return `true` 当物理设备符合所有要求时，反之返回 `false`
static bool is_physical_device_suitable(
    VkPhysicalDevice    physicalDevice, 
    VkSurfaceKHR        surface
)
{   
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    bool extensionsSupported = check_device_extension_properties(physicalDevice);

    QueueFamilyIndices queueFamilyIndices = 
        find_queue_families(physicalDevice, surface);

    bool swapchainSupported = false;
    if (extensionsSupported)
    {
        SwapchainSupportDetails swapchainSupportDetails = 
            query_swapchain_support_details(physicalDevice, surface);

        if (swapchainSupportDetails.formats != NULL 
            && swapchainSupportDetails.presentModes != NULL)
        {
            swapchainSupported = true;
        }

        free_swapchain_support_details(&swapchainSupportDetails);
    }

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU    // 是否独显
        && extensionsSupported                                  // 是否支持请求的扩展
        && queueFamilyIndices.graphicsSupport >= 0              // 是否队列支持图形
        && queueFamilyIndices.presentationSupport >= 0          // 是否队列族支持呈现
        && swapchainSupported;                  // 是否满足给定 Surface 的交换链创建要求
}

/// @brief 查询给定物理设备可用的扩展并打印出来，并检查请求的扩展是否可用
///
/// @return 当检查到有请求的扩展不可用时，该函数会打印相关信息，并返回 `false`
static bool check_device_extension_properties(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, NULL);

    fprintf(stdout,
        "%s: Found" ESC_FCOLOR_BRIGHT_GREEN " %u " ESC_RESET
        "available VkDevice extensions:\n",
        __func__, extensionCount);

    if (extensionCount < 1)
    {
        fprintf(stderr, 
            ESC_FCOLOR_RED
            "    No avaliable VkDevice extensions could be found!\n" ESC_RESET);
        return false;
    }

    VkExtensionProperties extensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(physicalDevice, 
        NULL, 
        &extensionCount, 
        extensions);

    for (int i = 0; i < extensionCount; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BRIGHT_GREEN "    %s\n" ESC_RESET,
            extensions[i].extensionName);
    }

    fprintf(stdout, "Application required device extensions:\n");

    uint32_t requiredDeviceExtensionCount = 
        sizeof(requiredDeviceExtensions) / sizeof(requiredDeviceExtensions[0]);
    for (int i = 0; i < requiredDeviceExtensionCount; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BLUE "    %s\n" ESC_RESET, requiredDeviceExtensions[i]);
    }

    bool hasOneNoFound = false;
    // (is subset) 判断一个数组是否是另一个数组的子集
    for (int i = 0; i < requiredDeviceExtensionCount; i++)
    {
        bool found = false;
        for (int j = 0; j < extensionCount; j++)
        {
            // strcmp 只有在两个字符串完全相等时才会返回 0（false）
            if (strcmp(requiredDeviceExtensions[i], extensions[j].extensionName))
                continue;

            found = true;
            break;
        }

        if (!found)
        {
            fprintf(stderr,
                    "Not supported device extension: %s,"
                    "can't found in available VkDevice extensions!\n",
                    requiredDeviceExtensions[i]);

            hasOneNoFound = true;
        }
    }

    return !hasOneNoFound;
}

static void dump_physical_device_properties(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    printf("--------------------------------------------------------------------\n");
    printf("Selected Physical Device: %s\n", properties.deviceName);
    printf("                    Type: %u\n", properties.deviceType);
    printf("          Driver Version: %u\n", properties.driverVersion);
    printf("               Vendor ID: %u\n", properties.vendorID);
    printf("--------------------------------------------------------------------\n");
}


VkDevice createLogicalDevice(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    VkQueue*            graphicsQueue,
    VkQueue*            presentationQueue
)
{
    int queueFamilyIndex = -1;
    bool useSingleQueue = 
        has_queue_family_supports_both_graphics_and_presentation(physicalDevice,
            surface,
            &queueFamilyIndex);
    
    VkDeviceQueueCreateInfo queueCreateInfo = {};   // 单队列族单队列

    VkDeviceQueueCreateInfo queueCreateInfoG = {};  // 双队列族双队列
    VkDeviceQueueCreateInfo queueCreateInfoP = {};  //
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    QueueFamilyIndices queueFamilyIndices = 
                find_queue_families(physicalDevice, surface);

    float queuePriorities = 1.0f;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    uint32_t requiredDeviceExtensionCount = 
        sizeof(requiredDeviceExtensions) / sizeof(requiredDeviceExtensions[0]);

    VkDeviceCreateInfo createInfo = {};

    if (useSingleQueue)
    {
        // 1.指定创建队列要用到的 VkDeviceQueueCreateInfo
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex    = queueFamilyIndex;
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &queuePriorities;

        // 2.指定 VkPhysicalDeviceFeatures
        // 默认

        // 3.指定 VkDeviceCreatInfo
        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos        = &queueCreateInfo;
        createInfo.queueCreateInfoCount     = 1;
        createInfo.pEnabledFeatures         = &deviceFeatures;
        createInfo.enabledExtensionCount    = requiredDeviceExtensionCount;
        createInfo.ppEnabledExtensionNames  = requiredDeviceExtensions;
    }
    else
    {
        // (GRAPHICS)
        queueCreateInfoG.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfoG.queueFamilyIndex   = queueFamilyIndices.graphicsSupport;
        queueCreateInfoG.queueCount         = 1;
        float queuePrioritiesG = 1.0f;
        queueCreateInfoG.pQueuePriorities   = &queuePrioritiesG;

        // (Presentation)
        queueCreateInfoP.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfoP.queueFamilyIndex   = queueFamilyIndices.presentationSupport;
        queueCreateInfoP.queueCount         = 1;
        float queuePrioritiesP = 1.0f;
        queueCreateInfoP.pQueuePriorities   = &queuePrioritiesP;

        queueCreateInfos[0] = queueCreateInfoG;
        queueCreateInfos[1] = queueCreateInfoP;

        // VkPhysicalDeviceFeatures
        // 默认

        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos        = queueCreateInfos;
        createInfo.queueCreateInfoCount     = 2;
        createInfo.pEnabledFeatures         = &deviceFeatures;
        createInfo.enabledExtensionCount    = requiredDeviceExtensionCount;
        createInfo.ppEnabledExtensionNames  = requiredDeviceExtensions;
    }

    // 4.创建逻辑设备
    VkDevice device = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(physicalDevice, &createInfo, NULL, &device);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, 
            "Failed to create a VkDevice! Error Code(VkResult): %d\n", result);
        
        return VK_NULL_HANDLE;
    }

    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "成功创建了一个 VkDevice！\n",
        __DATE__, __TIME__);

    // 5.out 参数形式返回创建好的 VkQueue
    if (useSingleQueue)
    {
        vkGetDeviceQueue(device, queueFamilyIndex, 0, graphicsQueue);
        vkGetDeviceQueue(device, queueFamilyIndex, 0, presentationQueue);
    }
    else
    {
        vkGetDeviceQueue(device, 
            queueFamilyIndices.graphicsSupport, 
            0, 
            graphicsQueue);
        vkGetDeviceQueue(device, 
            queueFamilyIndices.presentationSupport, 
            0, 
            presentationQueue);
    }

    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "获取了一个 VkQueue (for graphics)\n",
        __DATE__, __TIME__);

    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "获取了一个 VkQueue (for presentation)\n",
        __DATE__, __TIME__);

    fprintf(stdout, "Using Single Queue: ");
    if (useSingleQueue)
        fprintf(stdout, "true (Queue Family Index: %d)\n", queueFamilyIndex);
    else
        fprintf(stdout, "false\n");

    return device;
}


void destroyLogicalDevice(VkDevice device)
{
    vkDestroyDevice(device, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroyDevice！\n" ESC_RESET,
        __DATE__, __TIME__);
}


VkSwapchainKHR createSwapchain(
    GLFWwindow*         window,
    VkSurfaceKHR        surface,
    VkPhysicalDevice    physicalDevice, 
    VkDevice            device,
    uint32_t*           pSwapchainImageCount,   // 指向 uint32_t 变量的地址，用于输出
    VkImage**           ppSwapchainImages,      // 指向 VkImage 数组的地址，用于输出
    VkFormat*           pSwapchainImageFormat,  // 指向 VkFormat 变量的地址，用于输出
    VkExtent2D*         pSwapchainExtent        // 指向 VkExtent2D 变量的地址，用于输出
)
{
    if (window == NULL)
    {
        fprintf(stderr,
            "%s : 函数参数错误！传入了无效的 GLFWwindow 句柄！\n",
            __func__);

        return VK_NULL_HANDLE;
    }

    // 检查传入的地址是否为空
    if (pSwapchainImageCount == NULL 
        || ppSwapchainImages == NULL 
        || pSwapchainImageFormat == NULL
        || pSwapchainExtent == NULL)
    {
        fprintf(stderr,
            "%s : 函数参数错误！输出参数不能传入 NULL 地址！\n",
            __func__);

        return VK_NULL_HANDLE;
    }

    // 1.获取交换链支持信息
    SwapchainSupportDetails supportDetails = 
        query_swapchain_support_details(physicalDevice, surface);

    // 2.选择理想的 surface 格式、交换范围、交换链呈现模式和 image 数
    VkSurfaceFormatKHR surfaceFormat =
        get_optimal_surface_format(physicalDevice, surface);

    VkExtent2D extent = get_swap_exten(physicalDevice, surface, window);

    VkPresentModeKHR presentMode =
        get_optimal_prensent_mode(physicalDevice, surface);

    // 避免驱动等待，设置为 min + 1 个
    uint32_t minImageCount = supportDetails.capabilities.minImageCount + 1;
    // 限制 image 的数量（0 是特殊值，表没有最大值限制）
    if (supportDetails.capabilities.maxImageCount > 0)
    {
        minImageCount = minImageCount > supportDetails.capabilities.maxImageCount ?
            supportDetails.capabilities.maxImageCount : minImageCount;
    }

    // 3. 指定 VkSwapchainCreateInfoKHR
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = surface;

    createInfo.oldSwapchain             = VK_NULL_HANDLE;

    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageFormat              = surfaceFormat.format;
    createInfo.imageColorSpace          = surfaceFormat.colorSpace;
    createInfo.imageExtent              = extent;
    createInfo.presentMode              = presentMode;
    createInfo.minImageCount            = minImageCount;

    createInfo.preTransform             = supportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped                  = VK_TRUE;  // 启用窗口遮挡裁切

    // 3.5.处理 ImageSharingMode
    QueueFamilyIndices queueFamilyIndices = 
        find_queue_families(physicalDevice, surface);

    uint32_t pQueueFamilyIndices[] = 
        {queueFamilyIndices.graphicsSupport, queueFamilyIndices.presentationSupport};

    // 若使用单队列族单队列的
    if (has_queue_family_supports_both_graphics_and_presentation(physicalDevice,
            surface, NULL))
    {
       // 独占模式，一个 image 同时只能被一个队列族所有，跨队列族需要显式转移所有权
       createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    }
    // 使用双队列族双队列的
    else 
    {
       // 并发模式，一个 image 可以跨队列族使用，不需要显式转移所有权
       createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
       createInfo.queueFamilyIndexCount = 2;
       createInfo.pQueueFamilyIndices   = pQueueFamilyIndices;
    }

    // 4.创建交换链
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
            "Failed to create a VkSwapchainKHR! Error Code(VkResult): %d\n", result);

        return VK_NULL_HANDLE;
    }
    
    free_swapchain_support_details(&supportDetails);

    // 5.处理输出参数（交换链图像句柄数组和其大小、交换链图像格式和范围）
    uint32_t actualImageCount = 0;
    result = vkGetSwapchainImagesKHR(device, swapchain, &actualImageCount, NULL);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
            "Failed to get swapchain image count! Error Code(VkResult): %d\n", result);
        
        *pSwapchainImageCount = 0;
        *ppSwapchainImages = NULL;

        vkDestroySwapchainKHR(device, swapchain, NULL);     // 销毁刚刚创建的交换链

        return VK_NULL_HANDLE;
    }

    *pSwapchainImageCount = actualImageCount;

    // 为数组分配内存
    *ppSwapchainImages = (VkImage*)malloc(sizeof(VkImage) * actualImageCount);
    if (*ppSwapchainImages == NULL)
    {
        fprintf(stderr,
            ESC_FCOLOR_BRIGHT_RED "malloc err: "
            "Failed to malloc for swapchain image array!\n" ESC_RESET);

        *pSwapchainImageCount = 0;
        *ppSwapchainImages = NULL;

        vkDestroySwapchainKHR(device, swapchain, NULL);

        return VK_NULL_HANDLE;
    }

    result = vkGetSwapchainImagesKHR(device,
                swapchain,
                &actualImageCount, 
                *ppSwapchainImages);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
            "Failed to get swapchain images! Error Code(VkResult): %d\n", result);
        
        *pSwapchainImageCount = 0;
        free(*ppSwapchainImages);   // 释放数组内存
        *ppSwapchainImages = NULL;

        vkDestroySwapchainKHR(device, swapchain, NULL);

        return VK_NULL_HANDLE;
    }

    *pSwapchainImageFormat = surfaceFormat.format;
    *pSwapchainExtent = extent;

    fprintf(stdout,
        ESC_LTALIC "%s %s " ESC_RESET "成功创建了一个 VkSwapchainKHR！\n",
        __DATE__, __TIME__);

    return swapchain;
}


void destroySwapchain(VkDevice device, VkSwapchainKHR swapchain)
{
    vkDestroySwapchainKHR(device, swapchain, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroySwapchainKHR\n" ESC_RESET,
        __DATE__, __TIME__);
}