using System.Runtime.InteropServices;

namespace HelloTriangle.Nativelib;

/// <summary>
/// 动态链接库的托管包装类，见 <see cref="library"/>.
/// </summary>
public static partial class Windowing
{
    const string library = "nativelib_windowing";
    

    [LibraryImport(library, StringMarshalling = StringMarshalling.Utf8)]
    private static partial Window initializeWindow(int width, int height, string title);

    [LibraryImport(library)]
    private static partial void destroyWindow(Window window);

    [LibraryImport(library)]
    private static partial int windowShouldClose(Window window);

    [LibraryImport(library)]
    private static partial void pollEvents();

    [LibraryImport(library)]
    private static partial void terminate();

    /// <summary>
    /// 初始化 GLFW 库并创建一个窗口.
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="title"></param>
    /// <returns>一个创建好的 <see cref="Window"/> 对象，使用 <see cref="Window.IsInvalid"/> 来检查其是否有效</returns>
    public static Window InitializeWindow(int width, int height, string title)
    {
        return initializeWindow(width, height, title);
    }
    
    /// <summary>
    /// 销毁窗口.
    /// </summary>
    /// <param name="window">要销毁的窗口</param>
    public static void DestroyWindow(Window window)
    {
        destroyWindow(window);
    }

    /// <summary>
    /// 检查给定窗口是否 Close 标志位为 1.
    /// </summary>
    /// <param name="window">给定窗口</param>
    /// <returns><c>true</c> 如果窗口的 Close 标志位为 1.</returns>
    public static bool WindowShouldClose(Window window)
    {
        if (windowShouldClose(window) == 1)
            return true;

        return false;
    }

    /// <summary>
    /// 处理所有挂起的窗口事件.
    /// </summary>
    public static void PollEvents()
    {
        pollEvents();
    }

    /// <summary>
    /// 终止 GLFW 库.
    /// </summary>
    public static void Terminate()
    {
        terminate();
    }

}