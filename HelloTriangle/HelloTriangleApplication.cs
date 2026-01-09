using HelloTriangle.Windowing;
using HelloTriangle.Graphics;
using HelloTriangle.Graphics.Resources;

namespace HelloTriangle;

public class HelloTriangleApplication
{
    public void Run()
    {
        try
        {
            InitializeWindow();
            InitializeRenderer();
            MainLoop();
        }
        finally
        {
            CleanUp();
        }
    }

    private void InitializeWindow()
    {
        Window.Initialize(800, 600, "Vulkan");
        
        if (Window.Handle.IsInvalid)
            throw new InvalidOperationException("Failed to create a window.");
    }

    public void InitializeRenderer()
    {
        if (!Renderer.Initialize(Window.Handle))
            throw new InvalidOperationException("Failed to initialize renderer!");
    }

    private void MainLoop()
    {
        while (!Window.ShouldClose())
        {
            Window.PollEvents();
        }
    }

    private void CleanUp()
    {
        Console.WriteLine($"{nameof(CleanUp)}:");

        Renderer.Release();

        Window.Destroy();
        Window.Terminate();
    }
    
}