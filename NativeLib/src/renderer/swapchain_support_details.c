#include "swapchain_support_details.h"


SwapchainSupportDetails query_swapchain_support_details(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface    
)
{
    // 0.初始化
    SwapchainSupportDetails supportDetails = 
    {
        .formats = NULL,
        .presentModes = NULL
    };

    // 1.VkSurfaceCapabilitiesKHR
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, 
        surface, 
        &supportDetails.capabilities);

    // 2.VkSurfaceFormatKHR
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

    if (formatCount)
    {
        supportDetails.formats = (VkSurfaceFormatKHR*)
            malloc(formatCount * sizeof(VkSurfaceFormatKHR));

        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, 
            surface, 
            &formatCount, 
            supportDetails.formats);
    }
    else
    {
        fprintf(stderr, 
            ESC_FCOLOR_BRIGHT_RED 
            "No avaliable surface format could be found!\n" ESC_RESET);

        supportDetails.formats = NULL;
    }

    // 3.VkPresentModeKHR
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface, 
        &presentModeCount,
        NULL);

    if (presentModeCount)
    {
        supportDetails.presentModes = (VkPresentModeKHR*)
            malloc(presentModeCount * sizeof(VkPresentModeKHR));

        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface, 
        &presentModeCount,
        supportDetails.presentModes);
    }
    else
    {
        fprintf(stderr, 
            ESC_FCOLOR_BRIGHT_RED 
            "No avaliable surface present mode could be found!\n" ESC_RESET);

        supportDetails.presentModes = NULL;
    }

    return supportDetails;
}

void free_swapchain_support_details(SwapchainSupportDetails* pStructure)
{
    if(pStructure == NULL)
        return;

    if (pStructure->formats != NULL)
    {
        free(pStructure->formats);
        pStructure->formats = NULL;
    }

    if (pStructure->presentModes != NULL)
    {
        free(pStructure->presentModes);
        pStructure->presentModes = NULL;
    }
}

VkSurfaceFormatKHR get_optimal_surface_format(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
)
{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

    VkSurfaceFormatKHR surfaceFormats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, 
        surface, 
        &formatCount, 
        surfaceFormats);
    
    for (int i = 0; i < formatCount; i++)
    {
        // 理想选择为 B8G8R8A8_SRGB 和 SRGB_NONLINEAR_KHR
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormats[i];
        }
    }

    fprintf(stdout, 
        "物理设备不支持表面格式 B8G8R8A8_SRGB & SRGB_NONLINEAR_KHR，"
        "将使用第一个可用的表面格式！\n");

    return surfaceFormats[0];
}

VkPresentModeKHR get_optimal_prensent_mode(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
)
{
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface,
        &presentModeCount,
        NULL);

    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, 
        surface,
        &presentModeCount,
        presentModes);

    for (int i = 0; i < presentModeCount; i++)
    {
        // 理想选择为 VK_PRESENT_MODE_MAILBOX_KHR
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    fprintf(stdout, 
        "物理设备不支持 MAILBOX 呈现模式，"
        "将使用 FIFO 呈现模式！\n");

    return VK_PRESENT_MODE_FIFO_KHR;
}

static inline int clamp_int(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

VkExtent2D get_swap_exten(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    GLFWwindow*         window
)
{
    VkSurfaceCapabilitiesKHR capabilities = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, 
        surface, 
        &capabilities);

    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    // 若窗口管理器需要我们自行设置交换范围（ ^ 即高或宽是 uint32_t 的最大值）

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    width = clamp_int(width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width);

    height = clamp_int(height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);

    VkExtent2D actualExtent = {
        .width = width,
        .height = height
    };

    return actualExtent;
}