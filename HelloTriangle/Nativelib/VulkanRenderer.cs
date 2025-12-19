using System.Runtime.InteropServices;

namespace HelloTriangle.Nativelib;

/// <summary>
/// 动态链接库的托管包装类, 见 <see cref="library"/>.
/// </summary>
public static partial class VulkanRenderer
{
    const string library = "nativelib_vulkan_renderer";

    [LibraryImport(library)]
    private static partial VkInstance createInstance();

    [LibraryImport(library)]
    private static partial void destroyInstance(VkInstance instance);

    [LibraryImport(library)]
    private static partial VkPhysicalDevice pickPhysicalDevice(VkInstance instance);
    
    [LibraryImport(library)]
    private static partial VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice);

    [LibraryImport(library)]
    private static partial VkQueue getGraphicsQueue(VkPhysicalDevice physicalDevice, VkDevice device);

    [LibraryImport(library)]
    private static partial void destroyLogicalDevice(VkDevice device);

    /// <summary>
    /// 创建 VkInstance，其是程序和 Vulkan 库之间的接口.
    /// </summary>
    /// <returns>返回新创建的 VkInstance 句柄（当发生错误时返回空句柄）</returns>
    public static VkInstance CreateInstance()
    {
        return createInstance();
    }

    /// <summary>
    /// 销毁给定的 VkInstance.
    /// </summary>
    public static void DestroyInstance(VkInstance instance)
    {
        destroyInstance(instance);
    }

    /// <summary>
    /// 查询可用物理设备并尝试选择可用的显卡作 PhysicalDevice.
    /// </summary>
    /// <param name="instance">调用该函数需要传入一个 VkInstance 句柄</param>
    /// <returns> 返回一个可用的 PhysicalDevice 句柄（当发生错误时返回空句柄）</returns>
    public static VkPhysicalDevice PickPhysicalDevice(VkInstance instance)
    {
        return pickPhysicalDevice(instance);
    }

    /// <summary>
    /// 根据给定物理设备创建逻辑设备.
    /// </summary>
    /// <returns>返回新创建的 VkDevice 句柄（当发生错误时返回空句柄）</returns>
    public static VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice)
    {
        return createLogicalDevice(physicalDevice);
    }

    /// <summary>
    /// 在成功创建 VkDevice 后，调用该函数以获取（与 VkDevice 同时创建的）GRAPHICS 队列的句柄.
    /// </summary>
    /// <param name="physicalDevice">调用该函数需要传入一个 VkPhysicalDevice 句柄</param>
    /// <param name="device">调用该函数需要传入一个 VkDevice 句柄</param>
    /// <returns>传入 VkDevice 的 GRAPHICS 队列的句柄</returns>
    public static VkQueue GetGraphicsQueue(VkPhysicalDevice physicalDevice, VkDevice device)
    {
        return getGraphicsQueue(physicalDevice, device);
    }

    /// <summary>
    /// 销毁给定的 VkDevice.
    /// </summary>
    /// <param name="device"></param>
    public static void DestroyLogicalDevice(VkDevice device)
    {
        destroyLogicalDevice(device);
    }
    
}