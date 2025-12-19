using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkDevice 的托管 SafeHandle 包装.
/// <para>销毁：请使用 VulkanRenderer 中对应的 Destroy 函数而不是使用 Dispose！</para>
/// </summary>
public class VkDevice : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkDevice() : base(true) {}

    protected override bool ReleaseHandle()
    {
        return true;
    }
}