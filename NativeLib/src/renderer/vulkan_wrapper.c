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
static void get_driver_version_string(
    uint32_t    vendorID,
    uint32_t    driverVersion,
    uint32_t*   major,
    uint32_t*   minor,
    uint32_t*   patch
);
static uint32_t* read_spv_file(size_t* codeSize, const char* spvFilePath);


VkInstance createInstance(void)
{
    // 0.检查验证层是否开启并可用
    if (enableValidationLayers && !check_instance_layer_properties())
    {
        log_error("Validation layers requested, but not available!");

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

#ifdef DEBUG
    // 打印
    log_debug("GLFW required instance extensions:");
    for (int i = 0; i < glfwExtensionCount; i++)
    {
        log_debug(ESC_FCOLOR_BLUE "    %s" ESC_RESET,
            glfwExtensions[i]);
    }
#endif

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
        log_error("Failed to create a VkInstance! Error Code(VkResult): %d", result);

        return VK_NULL_HANDLE;
    }

    log_info("成功创建了一个 VkInstance！");

    return instance;
}

/// @brief 查询对 VkInstance 可用的层并打印出来，并检查请求的层是否可用.
///
/// @return 当检查到有请求的层不可用时，该函数会打印相关信息，并返回 `false`
static bool check_instance_layer_properties(void)
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    
    if (layerCount < 1)
    {
        log_error("No avaliable VkInstance layers could be found!");

        return false;
    }

    VkLayerProperties layers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, layers);

    uint32_t requiredValidationLayerCount = 
        sizeof(requiredValidationLayers) / sizeof(requiredValidationLayers[0]);

#ifdef DEBUG
    // 打印全部可用层名
    log_debug("%s: Found" ESC_FCOLOR_GREEN " %u " ESC_RESET
        "available VkInstance layers:",
        __func__, layerCount);
    for (int i = 0; i < layerCount; i++)
    {
        log_debug(ESC_FCOLOR_GREEN "    %s" ESC_RESET, layers[i].layerName);
    }
    // 打印我们请求的层名
    log_debug("Application required validation layers:");

    for (int i = 0; i < requiredValidationLayerCount ; i++)
    {
        log_debug(ESC_FCOLOR_BLUE "    %s" ESC_RESET, requiredValidationLayers[i]);
    }
#endif

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
            log_error("Not supported layer: %s, can't found in available layers!",
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

    if (extensionCount < 1)
    {
        log_error("No avaliable VkInstance extensions could be found!");

        return;
    }

#ifdef DEBUG
    VkExtensionProperties extensions[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    log_debug("%s: Found" ESC_FCOLOR_GREEN " %u " ESC_RESET
        "available VkInstance extensions:",
        __func__, extensionCount);
    for (int i = 0; i < extensionCount; i++)
    {
        log_debug(ESC_FCOLOR_GREEN "    %s" ESC_RESET, extensions[i].extensionName);
    }
#endif
}


void destroyInstance(VkInstance instance)
{   
    vkDestroyInstance(instance, NULL);

    log_trace("调用了 vkDestroyInstance！");
}


VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        log_error("Faild to create a VkSurfaceKHR! Error Code(VkResult): %d", result);

        return VK_NULL_HANDLE;
    }

    log_info("成功创建了一个 VkSurfaceKHR！");

    return surface;
}


void destroySurface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, NULL);

    log_trace("调用了 vkDestroySurfaceKHR！");
}


VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    // 1.查询可用的物理设备
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0)
    {
        log_error("Failed to find GPUs with Vulkan support!");

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
        log_error("Failed to find a suitable GPU!");

        return VK_NULL_HANDLE;
    }

    log_info("成功选取了一个物理设备！");

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

    if (extensionCount < 1)
    {
        log_error("No avaliable VkDevice extensions could be found!");

        return false;
    }

    VkExtensionProperties extensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(physicalDevice, 
        NULL, 
        &extensionCount, 
        extensions);

    uint32_t requiredDeviceExtensionCount = 
        sizeof(requiredDeviceExtensions) / sizeof(requiredDeviceExtensions[0]);

#ifdef DEBUG
    log_debug("%s: Found" ESC_FCOLOR_GREEN " %u " ESC_RESET
        "available VkDevice extensions:",
        __func__, extensionCount);
    for (int i = 0; i < extensionCount; i++)
    {
        log_debug(ESC_FCOLOR_GREEN "    %s" ESC_RESET,
            extensions[i].extensionName);
    }

    log_debug("Application required device extensions:");
    for (int i = 0; i < requiredDeviceExtensionCount; i++)
    {
        log_debug(ESC_FCOLOR_BLUE "    %s" ESC_RESET, requiredDeviceExtensions[i]);
    }
#endif

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
            log_error("Not supported device extension: %s,"
                "can't found in available VkDevice extensions!",
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
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    log_info("--------------------------------------------------------------------");
    log_info(" Selected Physical Device: %s", properties.deviceName);

    const char* deviceTypeStr = "Unknown";
    switch (properties.deviceType) 
    {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: 
            deviceTypeStr = "Integrated GPU"; 
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: 
            deviceTypeStr = "Discrete GPU"; 
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: 
            deviceTypeStr = "Virtual GPU"; 
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: 
            deviceTypeStr = "CPU"; 
            break;
        default: break;
    }
    log_info("                     Type: %s", deviceTypeStr);

    uint64_t totalMemory = 0;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) 
    {
        if (!(memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
            continue;

        totalMemory += memProperties.memoryHeaps[i].size;
    }
    log_info("Total VRAM (Device Local): %.3f GB",
        totalMemory / (1024.0f * 1024.0f * 1024.0f));

    uint32_t major, minor, patch;
    get_driver_version_string(properties.vendorID, 
        properties.driverVersion, 
        &major, &minor, &patch);
    log_info("    Vulkan Driver Version: %u.%u.%u", major, minor, patch);
    log_info("--------------------------------------------------------------------");
}

static void get_driver_version_string(
    uint32_t    vendorID,
    uint32_t    driverVersion,
    uint32_t*   major,
    uint32_t*   minor,
    uint32_t*   patch
)
{
    switch (vendorID)
    {
        case 0x10DE: // NVIDIA
        {
            *major = (driverVersion >> 22) & 0x3FF;
            *minor = (driverVersion >> 14) & 0xFF;
            *patch = (driverVersion >> 6) & 0xFF;
            break;
        }
        
        case 0x1002: // AMD
        {
            *major = VK_VERSION_MAJOR(driverVersion);
            *minor = VK_VERSION_MINOR(driverVersion);
            *patch = VK_VERSION_PATCH(driverVersion);
            break;
        }
        
        case 0x8086: // Intel
        {
            *major = VK_VERSION_MAJOR(driverVersion);
            *minor = VK_VERSION_MINOR(driverVersion);
            *patch = VK_VERSION_PATCH(driverVersion);
            break;
        }
        
        default: // 默认使用标准编码
        {
            *major = VK_VERSION_MAJOR(driverVersion);
            *minor = VK_VERSION_MINOR(driverVersion);
            *patch = VK_VERSION_PATCH(driverVersion);
            break;
        }
    }
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
        log_error("Failed to create a VkDevice! Error Code(VkResult): %d", result);

        return VK_NULL_HANDLE;
    }

    log_info("成功创建了一个 VkDevice！");

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

    log_info("获取了一个 VkQueue (for graphics).");
    log_info("获取了一个 VkQueue (for presentation).");

#ifdef DEBUG
    if (useSingleQueue)
        log_debug("Using Single Queue: true (Queue Family Index: %d)", queueFamilyIndex);
    else
        log_debug("Using Single Queue: false");
#endif

    return device;
}


void destroyLogicalDevice(VkDevice device)
{
    vkDestroyDevice(device, NULL);

    log_trace("调用了 vkDestroyDevice！");
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
    // 0.检查参数是否有效
    if (window == NULL)
    {
        log_error("%s : 函数参数错误！传入了无效的 GLFWwindow 句柄！", __func__);

        return VK_NULL_HANDLE;
    }

    // 检查传入的地址是否为空
    if (pSwapchainImageCount == NULL 
        || ppSwapchainImages == NULL 
        || pSwapchainImageFormat == NULL
        || pSwapchainExtent == NULL)
    {
        log_error("%s : 函数参数错误！输出参数不能传入 NULL 地址！", __func__);

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
        log_error("Failed to create a VkSwapchainKHR! Error Code(VkResult): %d",
            result);

        return VK_NULL_HANDLE;
    }

    free_swapchain_support_details(&supportDetails);

    // 5.处理输出参数（交换链图像句柄数组和其大小、交换链图像格式和范围）
    uint32_t actualImageCount = 0;
    result = vkGetSwapchainImagesKHR(device, swapchain, &actualImageCount, NULL);
    if (result != VK_SUCCESS)
    {
        log_error("Failed to get swapchain image count! Error Code(VkResult): %d",
            result);

        *pSwapchainImageCount = 0;
        *ppSwapchainImages = NULL;

        vkDestroySwapchainKHR(device, swapchain, NULL);     // 销毁刚刚创建的交换链

        return VK_NULL_HANDLE;
    }

    *pSwapchainImageCount = actualImageCount;

    // 为交换链图像句柄数组分配内存
    *ppSwapchainImages = (VkImage*)calloc(actualImageCount, sizeof(VkImage));
    if (*ppSwapchainImages == NULL)
    {
        log_error("%s : 交换链图像句柄数组内存分配失败！函数退出.", __func__);

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
        log_error("Failed to get swapchain images! Error Code(VkResult): %d", result);

        *pSwapchainImageCount = 0;
        free(*ppSwapchainImages);   // 释放数组内存
        *ppSwapchainImages = NULL;

        vkDestroySwapchainKHR(device, swapchain, NULL);

        return VK_NULL_HANDLE;
    }

    *pSwapchainImageFormat = surfaceFormat.format;
    *pSwapchainExtent = extent;

    log_info("成功创建了一个 VkSwapchainKHR！");

    return swapchain;
}


void destroySwapchain(
    VkDevice        device, 
    VkSwapchainKHR  swapchain, 
    VkImage**       ppSwapchainImages
)
{
    vkDestroySwapchainKHR(device, swapchain, NULL);

    // 释放在 createSwapchain() 中分配的交换链图像数组堆内存
    if(ppSwapchainImages)
    {
        free(*ppSwapchainImages);
        *ppSwapchainImages = NULL;
    }

    log_trace("调用了 vkDestroySwapchainKHR！");
}


VkImageView* createSwapchainImageViews(
    VkDevice        device,
    VkFormat        swapchainImageFormat,
    uint32_t        swapchainImageCount,
    const VkImage*  pSwapchainImages
)
{
    if (device == VK_NULL_HANDLE
        || swapchainImageCount == 0
        || pSwapchainImages == NULL)
    {
        log_error("%s : 传入了无效参数！无法为交换链图像创建视图.", __func__);

        return NULL;
    }

    // 1.分配交换链图像视图句柄数组
    VkImageView* pSwapchainImageViews = 
        (VkImageView*)calloc(swapchainImageCount, sizeof(VkImageView));
    if (pSwapchainImageViews == NULL)
    {
        log_error("%s : 交换链图像视图句柄数组内存分配失败！函数退出.", __func__);

        return NULL;
    }

    // 2.在循环体内创建 VkImageView
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image    = pSwapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format   = swapchainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;    // 通道默认映射
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;     // 不使用 mipmap
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;     // 单层

        VkResult result = vkCreateImageView(device, 
                              &createInfo, 
                              NULL, 
                              &pSwapchainImageViews[i]);
        if (result != VK_SUCCESS)
        {
            log_error("Failed to create VkImageViews(s, %u) for swapchain!"
                " Error Code(VkResult): %d",
                i, result);

            // 清理已创建的 VkImageView
            for (uint32_t j = 0; j < i; j++)
                vkDestroyImageView(device, pSwapchainImageViews[j], NULL);

            free(pSwapchainImageViews);   // 释放刚刚分配的数组

            return VK_NULL_HANDLE;
        }

        log_info("创建了一个 VkImageView(s, %u) for swapchain.", i);
    }

    return pSwapchainImageViews;
}


void destroySwapchainImageViews(
    VkDevice        device,
    uint32_t        swapchainImageCount,
    VkImageView**   ppSwapchainImageViews   // 要销毁的图像视图的数组的地址
)
{
    if (device == VK_NULL_HANDLE
        || swapchainImageCount == 0
        || ppSwapchainImageViews == NULL)
    {
        log_error("%s : 传入了无效参数！没有销毁任何交换链图像视图.", __func__);

        return;
    }

    // 遍历数组依次销毁
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyImageView(device, (*ppSwapchainImageViews)[i], NULL);

        log_trace("调用了 vkDestroyImageView(s，%u) for swapchian！", i);
    }

    // 释放数组占用的内存
    free(*ppSwapchainImageViews);
    *ppSwapchainImageViews = NULL;

    return;
}


VkRenderPass createRenderPass(
    VkDevice                        device,
    const VkRenderPassCreateInfo*   pCreateInfo
)
{
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkResult result = vkCreateRenderPass(device, pCreateInfo, NULL, &renderPass);
    if (result != VK_SUCCESS)
    {
        log_error("Failed to create a VkRenderPass! Error Code(VkResult): %d",
            result);

        return VK_NULL_HANDLE;
    }

    log_info("创建了一个 VkRenderPass.");

    return renderPass;
}


void destroyRenderPass(VkDevice device, VkRenderPass renderPass)
{
    vkDestroyRenderPass(device, renderPass, NULL);

    log_trace("调用了 vkDestroyRenderPass！");
}


VkFramebuffer createFramebuffer(VkDevice device, VkFramebufferCreateInfo* pCreateInfo)
{
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkResult result = vkCreateFramebuffer(device, pCreateInfo, NULL, &framebuffer);
    if (result != VK_SUCCESS)
    {
        log_error("Faild to create a VkFramebuffer! Error Code(VkResult): %d", result);

        return VK_NULL_HANDLE;
    }

    log_info("创建了一个 VkFramebuffer.");

    return framebuffer;
}


void destroyFramebuffer(VkDevice device, VkFramebuffer framebuffer)
{
    vkDestroyFramebuffer(device, framebuffer, NULL);

    log_trace("调用了 vkDestroyFramebuffer！");
}


VkShaderModule createShaderModule(
    VkDevice        device,
    const char*     spvFilePath
)
{
    size_t codeSize = 0;
    uint32_t* words = read_spv_file(&codeSize, spvFilePath);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode    = words;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);
    if (result != VK_SUCCESS)
    {
        log_error("Failed to create a VkShaderModule! Error Code(VkResult): %d",
            result);

        free(words);

        return VK_NULL_HANDLE;
    }

    free(words);    // 成功创建完 VkShaderModule 后就可以释放数组内存了

    log_info("创建了一个 VkShaderModule.");

    return shaderModule;
}

/// @brief 读取 spv 文件，返回的 spv 码数组记得释放.
///
/// @param codeSize 输出参数，返回 spv 文件的总大小（字节） 
/// @param spvFilePath spv 文件目录
///
/// @return spv 4 字节码数组，当发生错误时返回 `NULL`
static uint32_t* read_spv_file(size_t* codeSize, const char* spvFilePath)
{
    if (!codeSize || !spvFilePath)
    {
        log_error("%s(): 传入了无效参数！", __func__);
    }

    FILE* file = fopen(spvFilePath, "rb");     // 以二进制形式读打开
    if (!file)
    {
        log_error("%s(): Failed to open SPIR-V (.spv) file: %s", __func__, spvFilePath);
        
        return NULL;
    }

    // 获取文件大小（单位字节）
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 检查文件是否 4 字节对齐（SPIR-V 码规定）
    if (fileSize <= 0 || fileSize % 4 != 0)
    {
        log_error("%s(): " 
            "The given SPIR-V file size is invalid (must be a multiple of 4): %s",
            __func__, spvFilePath);

        fclose(file);

        return NULL;
    }

    size_t wordCount = fileSize / 4;

    // 分配数组
    uint32_t* words = (uint32_t*)calloc(wordCount, sizeof(uint32_t)); 
    if (!words)
    {
        log_error("%s(): 数组内存分配失败！", __func__);
        
        fclose(file);

        return NULL;
    }

    // 读文件，以 4 字节为单位
    size_t readCount = fread(words, sizeof(uint32_t), wordCount, file);
    
    fclose(file);

    // 错误检查
    if (readCount != wordCount)
    {
        log_error("%s(): SPIR-V 文件读取不完整！", __func__);

        free(words);

        return NULL;
    }

    // 错误检查
    if (words[0] != 0x07230203u)
    {
        log_error("%s(): 读取的 SPIR-V 文件无效，第一位码不等于 0x07230203：0x%08x",
            __func__, words[0]);

        free(words);

        return NULL;
    }

    *codeSize = fileSize;

    return words;
}


void destroyShaderModule(VkDevice device, VkShaderModule shaderModule)
{
    vkDestroyShaderModule(device, shaderModule, NULL);

    log_trace("调用了 vkDestroyShaderModule！");
}


VkPipelineLayout createPipelineLayout(
    VkDevice                            device,
    const VkPipelineLayoutCreateInfo*   pCreateInfo
)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkResult result = vkCreatePipelineLayout(device,
                          pCreateInfo,
                          NULL,
                          &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        log_error("Failed to create a VkPipelineLayout! Error Code(VkResult): %d",
            result);

        return VK_NULL_HANDLE;
    }

    log_info("创建了一个 VkPipelineLayout.");

    return pipelineLayout;
}


void destroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout)
{
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);

    log_trace("调用了 vkDestroyPipelineLayout！");
}


VkPipeline createGraphicsPipeline(
    VkDevice                             device,
    const VkGraphicsPipelineCreateInfo*  pCreateInfo
)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result = vkCreateGraphicsPipelines(device,
                          VK_NULL_HANDLE,
                          1,
                          pCreateInfo,
                          NULL,
                          &pipeline);
    if (result != VK_SUCCESS)
    {
        log_error("Failed to create a VkPipeline(Graphics)! Error Code(VkResult): %d",
            result);

        return VK_NULL_HANDLE;
    }

    log_info("创建了一个 VkPipeline.");

    return pipeline;
}


void destroyPipeline(VkDevice device, VkPipeline pipeline)
{
    vkDestroyPipeline(device, pipeline, NULL);

    log_trace("调用了 vkDestroyPipeline!");
}

