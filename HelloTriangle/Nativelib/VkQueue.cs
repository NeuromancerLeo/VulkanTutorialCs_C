using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkQueue 的托管 SafeHandle 包装.
/// </summary>
public class VkQueue : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkQueue() : base(true) {}

    protected override bool ReleaseHandle()
    {
        if(handle == IntPtr.Zero)
            return false;
        
        // VkQueue 在销毁 VkDevice 时隐式销毁，不需要手动销毁
        handle = -1;

        return true;
        
    }
}