## 如何编译该项目

该项目在 64 位的 Windows 和 Linux 系统下均可正常编译并工作:

_（在 Windows 下该项目使用 MinGW-w64 工具链进行本地代码部分的编译，我使用的版本为：`(MinGW-W64 x86_64-msvcrt-posix-seh, built by Brecht Sanders), r4 15.1.0`，若要使用其他编译器如 MSVC，需前往项目的 NativeLib 文件夹下修改 `xmake.lua` 文件）_

#### 1. 下载 [.NET 10.0 SDK](https://dotnet.microsoft.com/zh-cn/download/dotnet/10.0) 并安装

#### 2. 下载 [Vulkan SDK](https://vulkan.lunarg.com/) 并安装

#### 3. 下载 [Xmake](https://xmake.io/) 并安装

#### 4. 在确保相关环境变量配置完成后

1. 在项目根目录下控制台输入 `cd NativeLib`（`xmake.lua` 所在目录）；
2. 然后依次输入 `xmake config -m release` 和 `xmake build` 编译项目的本地代码部分，接着输入 `xmake install -o ./` 编译第三方库；
3. 回到项目根目录，然后前往 `Resources/Shaders`，在该目录下新建 `bin` 文件夹，并运行 `./compile.dat`（Linux 则运行 `bash ./compile.sh`）以编译着色器文件；
4. 回到项目根目录，控制台输入 `dotnet build --Configuration=Release` 构建项目的 C# 部分；
5. 输入 `cd HelloTriangle/bin/x64/Release/net10.0` 前往构建输出目录，然后输入 `./HelloTriangle` 即可运行程序.

## 一些运行成果截图

顶点绘制 + 索引式绘制：
![顶点绘制 + 索引式绘制](./READMEImages/VertexDraw%20+%20IndexedDraw.png)
