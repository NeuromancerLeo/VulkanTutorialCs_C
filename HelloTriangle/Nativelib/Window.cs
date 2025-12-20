using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// GLFWwindow 的托管 SafeHandle 包装.
/// <para>销毁：请使用 <see cref="Windowing"/> 中对应的 Destroy 函数而不是使用 Dispose！</para>
/// </summary>
public class Window : SafeHandleZeroOrMinusOneIsInvalid
{
    public Window() : base(true) { }

    protected override bool ReleaseHandle()
    {
        return true;
    }
}