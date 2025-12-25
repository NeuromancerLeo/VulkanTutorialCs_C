//该类会实现 IRenderer 接口，重要的是，该类不会暴露任何图形 API 的细节

using System.Runtime.InteropServices;
using HelloTriangle.Nativelib;

namespace HelloTriangle;

public static partial class Renderer
{
    const string library = "nativelib_renderer";

    [LibraryImport(library)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static partial bool rendererInitialize(Window window);

    [LibraryImport(library)]
    private static partial void rendererReady();

    [LibraryImport(library)]
    private static partial void rendererBeginFrame();

    [LibraryImport(library)]
    private static partial void rendererEndFrame();

    [LibraryImport(library)]
    private static partial void rendererRelease();


    public static bool Initialize(Window window)
    {
        return rendererInitialize(window);
    }

    public static void Ready()
    {
        rendererReady();
    }

    public static void BegineFrame()
    {
        rendererBeginFrame();
    }

    public static void EndFrame()
    {
        rendererEndFrame();
    }

    public static void Release()
    {
        rendererRelease();
    }
}