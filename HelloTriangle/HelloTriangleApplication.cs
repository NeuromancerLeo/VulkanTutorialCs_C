using HelloTriangle.Windowing;
using HelloTriangle.Graphics;

namespace HelloTriangle;

public class HelloTriangleApplication
{
    public void Run()
    {
        try
        {
            Initialize();
            Ready();
            MainLoop();
        }
        finally
        {
            CleanUp();
        }
    }

    private void Initialize()
    {
        InitializeWindow();
        InitializeRenderer();
    }

    private void InitializeWindow()
    {
        Window.Initialize(800, 600, "Vulkan");
        
        if (Window.Handle.IsInvalid)
            throw new InvalidOperationException("Failed to create a window.");
    }

    private void InitializeRenderer()
    {
        if (!Renderer.Initialize(Window.Handle))
            throw new InvalidOperationException("Failed to initialize renderer!");
    }

    private void Ready()
    {
        if (!Renderer.Ready())
            throw new InvalidOperationException("An error occurred when calling Renderer.Ready()!");
    }

    private void MainLoop()
    {
        while (!Window.ShouldClose())
        {
            Window.PollEvents();

            Renderer.DrawFrame();
        }
    }

    private void CleanUp()
    {
        Console.WriteLine($"{nameof(HelloTriangleApplication)}.{nameof(CleanUp)}():");

        Renderer.Release();

        Window.Destroy();
        Window.Terminate();
    }
    
}