using System.Runtime.InteropServices;
using HelloTriangle.Nativelib;

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
        Windowing.InitializeWindow(800, 600, "Vulkan");
        
        if (Windowing.Handle.IsInvalid)
            throw new InvalidOperationException("Failed to create a window.");
    }

    public void InitializeRenderer()
    {
        if (!Renderer.Initialize(Windowing.Handle))
            throw new InvalidOperationException("Failed to initialize renderer!");
    }

    private void MainLoop()
    {
        while (!Windowing.WindowShouldClose())
        {
            Windowing.PollEvents();
        }
    }

    private void CleanUp()
    {
        Console.WriteLine($"{nameof(CleanUp)}:");

        Renderer.Release();

        Windowing.DestroyWindow();
        Windowing.Terminate();
    }
    
}