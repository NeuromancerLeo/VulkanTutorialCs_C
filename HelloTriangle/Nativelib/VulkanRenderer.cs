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
    private static partial VkSurfaceKHR createSurface(VkInstance instance, 
                                                      Window window);

    [LibraryImport(library)]
    private static partial void destroySurface(VkInstance instance,
                                               VkSurfaceKHR surface);

    [LibraryImport(library)]
    private static partial VkPhysicalDevice pickPhysicalDevice(VkInstance instance, 
                                                               VkSurfaceKHR surface);
    
    [LibraryImport(library)]
    private static partial VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        out VkQueue graphicsQueue,
                                                        out VkQueue presentationQueue);

    [LibraryImport(library)]
    private static partial void destroyLogicalDevice(VkDevice device);

    [LibraryImport(library)]
    private static partial VkSwapchainKHR createSwapchain(Window window,
                                                          VkSurfaceKHR surface,
                                                          VkPhysicalDevice physicalDevice,
                                                          VkDevice device);

    [LibraryImport(library)]
    private static partial void destroySwapchain(VkDevice device,
                                                 VkSwapchainKHR swapchain);

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
    /// 在成功创建 VkInstance 后调用该函数创建 VkSurfaceKHR，其用于将渲染的图形呈现到屏幕上.
    /// </summary>
    /// <param name="instance">
    /// 调用该函数需要传入一个 VkInstance 句柄
    /// （需确保已启用了 `VK_KHR_surface` 扩展和平台相关的扩展如 `VK_KHR_win32_surface`）
    /// </param>
    /// <param name="window">调用该函数需要传入一个 GLFWwindow 句柄</param>
    /// <returns>返回新创建的 VkSurfaceKHR 句柄（当发生错误时返回空句柄）</returns>
    public static VkSurfaceKHR CreateSurface(VkInstance instance, Window window)
    {
        return createSurface(instance, window);
    }

    /// <summary>
    /// 销毁给定的 VkSurfaceKHR.
    /// </summary>
    /// <param name="instance">调用该函数需要传入一个 VkInstance 句柄</param>
    public static void DestroySurface(VkInstance instance, VkSurfaceKHR surface)
    {
        destroySurface(instance, surface);
    }

    /// <summary>
    /// 查询可用物理设备并尝试选择可用的显卡作 PhysicalDevice.
    /// </summary>
    /// <param name="instance">调用该函数需要传入一个 VkInstance 句柄</param>
    /// <param name="surface">调用该函数需要传入一个 VkSurfaceKHR 句柄</param>
    /// <returns> 返回一个可用的 PhysicalDevice 句柄（当发生错误时返回空句柄）</returns>
    public static VkPhysicalDevice PickPhysicalDevice(VkInstance instance,
                                                      VkSurfaceKHR surface)
    {
        return pickPhysicalDevice(instance, surface);
    }

    /// <summary>
    /// 根据给定物理设备创建逻辑设备.
    /// </summary>
    /// <returns>返回新创建的 VkDevice 句柄（当发生错误时返回空句柄）</returns>
    public static VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice,
                                               VkSurfaceKHR surface,
                                               out VkQueue graphicsQueue,
                                               out VkQueue presentationQueue)
    {
        return createLogicalDevice(
                physicalDevice, 
                surface, 
                out graphicsQueue, 
                out presentationQueue);
    }

    /// <summary>
    /// 销毁给定的 VkDevice.
    /// </summary>
    /// <param name="device"></param>
    public static void DestroyLogicalDevice(VkDevice device)
    {
        destroyLogicalDevice(device);
    }
    
    /// <summary>
    /// 根据给定窗口句柄和设备创建交换链.
    /// </summary>
    /// <param name="window">给定窗口句柄</param>
    /// <param name="surface">给定 Surface 句柄</param>
    /// <param name="physicalDevice">给定物理设备句柄</param>
    /// <param name="device">给定设备句柄</param>
    /// <returns>返回新创建的 VkSwapchainKHR 句柄（当发生错误时返回空句柄）</returns>
    public static VkSwapchainKHR CreateSwapchain(Window window, 
                                                 VkSurfaceKHR surface,
                                                 VkPhysicalDevice physicalDevice,
                                                 VkDevice device)
    {
        return createSwapchain(window, surface, physicalDevice, device);
    }

    /// <summary>
    /// 销毁给定的 VkSwapchainKHR.
    /// </summary>
    /// <param name="device">调用该函数需要传入一个对应的 VkDevice 句柄</param>
    public static void DestroySwapchain(VkDevice device, VkSwapchainKHR swapchain)
    {
        destroySwapchain(device, swapchain);
    }
}