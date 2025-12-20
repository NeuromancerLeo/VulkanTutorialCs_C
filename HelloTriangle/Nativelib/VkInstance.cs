using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkInstance 的托管 SafeHandle 包装.
/// <para>销毁：请使用 <see cref="VulkanRenderer"/> 中对应的 Destroy 函数而不是使用 Dispose！</para>
/// </summary>
public class VkInstance : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkInstance() : base(true) {}

    protected override bool ReleaseHandle()
    {
        return true;
    }
}