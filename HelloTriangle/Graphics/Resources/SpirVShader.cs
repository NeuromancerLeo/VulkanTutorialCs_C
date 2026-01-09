using System.Diagnostics;

namespace HelloTriangle.Graphics.Resources;

/// <summary>
/// 该类表示 <seealso cref="Renderer"/> 在创建一个图形管线时所需要用到的 SPIR-V 着色器模块
/// </summary>
public class SpirVShader
{
    private readonly string entryPoint = "";
    private readonly uint[] words;

    /// <summary>
    /// 该着色器模块的入口函数名
    /// </summary>
    public string EntryPoint { get => entryPoint; }

    /// <summary>
    /// 4 字节对齐的 SPIR-V 码数组.
    /// </summary>
    public uint[] Words { get => words; }

    /// <summary>
    /// 构造函数，创建该对象需要指定 .spv 文件路径和其入口函数名.
    /// </summary>
    /// <param name="spvFilePath">.spv 二进制文件路径</param>
    /// <param name="entryPoint">该着色器模块的入口函数名</param>
    /// <exception cref="FileNotFoundException"></exception>
    public SpirVShader(string spvFilePath, string entryPoint)
    {
        if (!File.Exists(spvFilePath))
            throw new FileNotFoundException(".spv shader file not found!", spvFilePath);

        byte[] rawBytes = File.ReadAllBytes(spvFilePath);  // 加载 SPIR-V 文件
        words = new uint[rawBytes.Length / 4];
        // 该方法是按字节复制的
        Buffer.BlockCopy(rawBytes, 0, words, 0, rawBytes.Length);

        this.entryPoint = entryPoint;
    }
}