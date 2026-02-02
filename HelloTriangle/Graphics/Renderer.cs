//该类会实现 IRenderer 接口，重要的是，该类不会暴露任何图形 API 的细节

using System.Runtime.InteropServices;
using HelloTriangle.Windowing;

namespace HelloTriangle.Graphics;

public static partial class Renderer
{
    const string library = "nativelib_renderer";

    [LibraryImport(library)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static partial bool rendererInitialize(GLFWwindowSafeHandle window);

    [LibraryImport(library)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static partial bool rendererReady();

    [LibraryImport(library)]
    private static partial void rendererDrawFrame();

    [LibraryImport(library)]
    private static partial void rendererRelease();


    public static bool Initialize(GLFWwindowSafeHandle window)
    {
        return rendererInitialize(window);
    }

    public static bool Ready()
    {
        return rendererReady();
    }

    public static void DrawFrame()
    {
        rendererDrawFrame();
    }

    public static void Release()
    {
        rendererRelease();
    }
}