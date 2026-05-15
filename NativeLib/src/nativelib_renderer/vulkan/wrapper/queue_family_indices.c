#include "queue_family_indices.h"


void find_queue_families(
    VkPhysicalDevice    physicalDevice,
    VkSurfaceKHR        surface,
    QueueFamilyIndices* pFamilyIndices
)
{
    if (!pFamilyIndices)
    {
        log_error("%s(): 输出参数传入了无效地址，函数退出.", __func__);

        return;
    }

    // 初始化所有索引字段为 -1（该值表示未找到可用的族索引）
    memset(pFamilyIndices, 0xFF, sizeof(QueueFamilyIndices));

    // 1.传入物理设备查询其队列族 Properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount, 
        NULL);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount,
        queueFamilies);
    
    // 2.遍历队列族 Properties（按最后支持相关功能 BIT 的族来填索引）
    for (int i = 0; i < queueFamilyCount; i++)
    {
        // 如果队列族一个队列都没有
        if (queueFamilies[i].queueCount < 1)
            continue;
        
        // 检查其队列 flags
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            pFamilyIndices->graphicsSupport = i;

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            pFamilyIndices->transferSupport = i;

        // 检查其是否支持呈现
        VkBool32 supportsPresentation = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                              i,
                              surface,
                              &supportsPresentation);
        if (result != VK_SUCCESS)
        {
            log_error("%s(): An error occurred when calling "
                "vkGetPhysicalDeviceSurfaceSupportKHR()! "
                "Error Code(VkResult): %d", result);

            continue;
        }

        if (supportsPresentation == VK_TRUE)
            pFamilyIndices->presentationSupport = i;
    }
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