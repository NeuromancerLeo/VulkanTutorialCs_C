#include "queue_family_indices.h"


static QueueFamilyIndices init_QueueFamilyIndices()
{
    // 初始化，-1 表未找到
    QueueFamilyIndices queueFamilyIndices = 
    {
        .graphicsSupport        = -1,   
        .presentationSupport    = -1
    };

    return queueFamilyIndices;
}

QueueFamilyIndices find_queue_families(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface
)
{
    QueueFamilyIndices queueFamilyIndices = init_QueueFamilyIndices();

    // 1.查询队列族 Properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount, 
        NULL);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount,
        queueFamilies);
    
    // 2.遍历队列族 Properties
    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueCount < 1)
            continue;
        
        // 检查其队列 flags
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsSupport = i;

        // 检查其是否支持呈现
        VkBool32 supportsPresentation = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
            i,
            surface,
            &supportsPresentation);
        if (supportsPresentation == VK_TRUE)
            queueFamilyIndices.presentationSupport = i;
    }

    return queueFamilyIndices;

}

bool has_queue_family_supports_both_graphics_and_presentation(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    int*                queueFamilyIndex
)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount, 
        NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount,
        queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueCount < 1)    // 我想不通有什么b显卡有队列族没队列的
            continue;

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 supportsPresentation = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                i,
                surface,
                &supportsPresentation);
            
            // 找到符合条件的马上设置传入队列族索引并返回 true
            if (supportsPresentation == VK_TRUE)
            {
                if (queueFamilyIndex != NULL)
                    *queueFamilyIndex = i;

                return true;
            }
        }
    }

    if (queueFamilyIndex != NULL)
        *queueFamilyIndex = -1;     // 设为 -1 表未找到
    
        return false;
}