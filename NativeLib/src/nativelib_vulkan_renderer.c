#include "nativelib_vulkan_renderer.h"
#include "queue_family_indices.h"
#include "ansiecs.h"


EX_API VkInstance createInstance(void)
{
    // 0.检查验证层是否开启并可用
    if (enableValidationLayers && !check_instance_layer_support_properties())
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
        uint32_t validationLayersCount = 
            sizeof(validationLayers) / sizeof(validationLayers[0]);

        createInfo.enabledLayerCount    = validationLayersCount;
        createInfo.ppEnabledLayerNames  = validationLayers;
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

bool check_instance_layer_support_properties(void)
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
    
    uint32_t validationLayersCount = 
        sizeof(validationLayers) / sizeof(validationLayers[0]);
    for (int i = 0; i < validationLayersCount ; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BLUE "    %s\n" ESC_RESET, validationLayers[i]);
    }

    // 检查 validationLayer 中的层是否可用
    for (int i = 0; i < validationLayersCount; i++)
    {
        bool layerFound = false;
        for (int j = 0; j < layerCount; j++)
        {
            if (!strcmp(validationLayers[i], layers[j].layerName))
                continue;

            layerFound = true;
            break;
        }
        
        // 只要有一个找不到就返回 false
        if(!layerFound)
        {
            fprintf(stderr,
                "Not supported layer: %s, can't found in available layers!",
                validationLayers[i]);
            return false;
        }
    }

    return true;
}

void check_instance_extension_properties(void)
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


EX_API void destroyInstance(VkInstance instance)
{
    if (instance == VK_NULL_HANDLE) return;
    
    vkDestroyInstance(instance, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroyInstance！\n" ESC_RESET,
        __DATE__, __TIME__);
}


EX_API VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
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


EX_API void destroySurface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroySurfaceKHR！\n" ESC_RESET,
        __DATE__, __TIME__);
}


EX_API VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
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

bool is_physical_device_suitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{   
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    bool extensionsSupported = 
        check_device_extension_properties(physicalDevice);

    QueueFamilyIndices queueFamilyIndices = 
        find_queue_families(physicalDevice, surface);

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && extensionsSupported
        && queueFamilyIndices.graphicsSupport >= 0 
        && queueFamilyIndices.presentationSupport >= 0;
}

bool check_device_extension_properties(VkPhysicalDevice physicalDevice)
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

    uint32_t requiredDeviceExtensionsCount = 
        sizeof(requiredDeviceExtensions) / sizeof(requiredDeviceExtensions[0]);
    for (int i = 0; i < requiredDeviceExtensionsCount ; i++)
    {
        fprintf(stdout,
            ESC_FCOLOR_BLUE "    %s\n" ESC_RESET, requiredDeviceExtensions[i]);
    }

    // TODO: is_subset 判断一个数组是否是另一个数组的子集

    return false;
}

void dump_physical_device_properties(VkPhysicalDevice physicalDevice)
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


EX_API VkDevice createLogicalDevice(
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
        createInfo.enabledExtensionCount    = 0;
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
        createInfo.enabledExtensionCount    = 0;
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


EX_API void destroyLogicalDevice(VkDevice device)
{
    vkDestroyDevice(device, NULL);

    fprintf(stdout, 
        ESC_LTALIC "%s %s " ESC_RESET
        ESC_FCOLOR_BRIGHT_MAGENTA "调用了 vkDestroyDevice！\n" ESC_RESET,
        __DATE__, __TIME__);
}