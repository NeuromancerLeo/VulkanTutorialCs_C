using System.Runtime.InteropServices;
using HelloTriangle.Nativelib;

namespace HelloTriangle;

public class HelloTriangleApplication
{
    Window window = null!;

    VkInstance instance = null!;
    VkSurfaceKHR surface = null!;
    VkPhysicalDevice physicalDevice = null!;
    VkDevice device = null!;
    VkQueue graphicsQueue = null!;
    VkQueue presentationQueue = null!;

    public void Run()
    {
        InitializeWindow();
        InitializeVulkan();
        MainLoop();
        CleanUp();
    }

    private void InitializeWindow()
    {
        window = Windowing.InitializeWindow(800, 600, "Vulkan");
        
        if (window.IsInvalid)
            throw new InvalidOperationException("Failed to create a window.");
    }

    private void InitializeVulkan()
    {
        instance = VulkanRenderer.CreateInstance();
        if (instance.IsInvalid) 
            throw new InvalidOperationException("Failed to create a VkInstance.");

        surface = VulkanRenderer.CreateSurface(instance, window);
        if (surface.IsInvalid)
            throw new InvalidOperationException("Failed to create a VkSurfaceKHR.");

        physicalDevice = VulkanRenderer.PickPhysicalDevice(instance, surface);
        if (physicalDevice.IsInvalid)
            throw new InvalidOperationException("Failed to pick a VkPhysicalDevice.");

        device = VulkanRenderer.CreateLogicalDevice(physicalDevice, surface, out graphicsQueue, out presentationQueue);
        if (device.IsInvalid)
            throw new InvalidOperationException("Failed to create a VkDevice.");
        if (graphicsQueue.IsInvalid)
            throw new InvalidOperationException("Failed to get a VkQueue (for graphics).");
        if (presentationQueue.IsInvalid)
            throw new InvalidOperationException("Failed to get a VkQueue (for presentation).");
        
    }
  
    private void MainLoop()
    {
        while (!Windowing.WindowShouldClose(window))
        {
            Windowing.PollEvents();
        }
    }

    private void CleanUp()
    {
        Console.WriteLine($"{nameof(CleanUp)}:");

        VulkanRenderer.DestroyLogicalDevice(device);
        VulkanRenderer.DestroySurface(instance, surface);
        VulkanRenderer.DestroyInstance(instance);
        
        window.Dispose();
        Windowing.Terminate();
    }
    
}