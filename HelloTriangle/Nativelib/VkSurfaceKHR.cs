using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkQueue 的托管 SafeHandle 包装.
/// <para>销毁：请使用 VulkanRenderer 中对应的 Destroy 函数而不是使用 Dispose！</para>
/// </summary>
public class VkSurfaceKHR : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkSurfaceKHR() : base(true) {}

    protected override bool ReleaseHandle()
    {
        return true;
    }
}