using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// GLFWwindow 的托管 SafeHandle 包装.
/// </summary>
public class Window : SafeHandleZeroOrMinusOneIsInvalid
{
    public Window() : base(true) { }

    protected override bool ReleaseHandle()
    {
        if(handle == IntPtr.Zero)
            return false;

        Windowing.DestroyWindow(this);

        return true;
    }
}