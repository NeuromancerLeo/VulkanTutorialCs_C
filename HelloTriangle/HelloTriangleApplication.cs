using System.Runtime.InteropServices;
using HelloTriangle.Nativelib;

namespace HelloTriangle;

public class HelloTriangleApplication
{
    Window window = null!;

    VkInstance instance = null!;
    VkPhysicalDevice physicalDevice = null!;
    VkDevice device = null!;
    VkQueue graphicsQueue = null!;

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

        physicalDevice = VulkanRenderer.PickPhysicalDevice(instance);
        if (physicalDevice.IsInvalid)
            throw new InvalidOperationException("Failed to pick a VkPhysicalDevice.");

        device = VulkanRenderer.CreateLogicalDevice(physicalDevice);
        if (device.IsInvalid)
            throw new InvalidOperationException("Failed to create a VkDevice.");

        graphicsQueue = VulkanRenderer.GetGraphicsQueue(physicalDevice, device);
        if (graphicsQueue.IsInvalid)
            throw new InvalidOperationException("Failed to get a VkQueue.");
        
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
        VulkanRenderer.DestroyLogicalDevice(device);
        VulkanRenderer.DestroyInstance(instance);
        
        window.Dispose();
        Windowing.Terminate();
    }
    
}