using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkInstance 的托管 SafeHandle 包装.
/// </summary>
public class VkInstance : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkInstance() : base(true) {}

    protected override bool ReleaseHandle()
    {
        if(handle == IntPtr.Zero)
            return false;
        
        VulkanRenderer.DestroyInstance(this);

        return true;
        
    }
}