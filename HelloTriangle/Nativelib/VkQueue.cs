using Microsoft.Win32.SafeHandles;

namespace HelloTriangle.Nativelib;

/// <summary>
/// VkQueue 的托管 SafeHandle 包装.
/// <para>销毁：VkQueue 在销毁 VkDevice 时隐式销毁，不需要手动销毁.</para>
/// </summary>
public class VkQueue : SafeHandleZeroOrMinusOneIsInvalid
{
    public VkQueue() : base(true) {}

    protected override bool ReleaseHandle()
    {
        return true;
    }
}