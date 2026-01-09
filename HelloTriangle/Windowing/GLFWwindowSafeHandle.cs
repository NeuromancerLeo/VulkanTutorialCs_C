using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Windowing;

/// <summary>
/// GLFWwindow 的托管 SafeHandle 包装.
/// <para>销毁：请使用 <see cref="Window"/> 中对应的 Destroy 函数而不是使用 Dispose！</para>
/// </summary>
public class GLFWwindowSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
{
    public GLFWwindowSafeHandle() : base(true) { }

    protected override bool ReleaseHandle()
    {
        return true;
    }
}