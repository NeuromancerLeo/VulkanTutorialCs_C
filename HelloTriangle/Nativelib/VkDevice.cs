using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkDevice 的托管 SafeHandle 包装.
/// </summary>
public class VkDevice : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkDevice() : base(true) {}

    protected override bool ReleaseHandle()
    {
        if(handle == IntPtr.Zero)
            return false;
        
        // TODO destroy...

        return true;
        
    }
}