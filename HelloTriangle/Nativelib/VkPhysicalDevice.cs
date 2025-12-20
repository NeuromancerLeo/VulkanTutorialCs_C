using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkPhysicalDevice 的托管 SafeHandle 包装.
/// <para>销毁：VkPhysicalDevice 在销毁 VkInstance 时隐式销毁，不需要手动销毁</para>
/// </summary>
public class VkPhysicalDevice : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkPhysicalDevice() : base(true) {}

    protected override bool ReleaseHandle()
    {
        return true;
    }
}