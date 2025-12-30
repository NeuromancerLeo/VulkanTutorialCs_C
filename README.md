## 如何编译该项目

#### 1. 下载 [.NET 10.0 SDK](https://dotnet.microsoft.com/zh-cn/download/dotnet/10.0) 并安装
#### 2. 下载 [Vulkan SDK](https://vulkan.lunarg.com/) 并安装
#### 3. 下载 [Xmake](https://xmake.io/) 并安装
#### 4. 在确保相关环境变量配置完成后
 在项目根目录下控制台输入 `cd NativeLib`（`xmake.lua` 所在目录），然后输入 `xmake build` 编译项目的本地代码部分，接着输入 `xmake install -o ./` 编译第三方库，前往生成的 bin 目录下复制里面所有的 .dll \ .so 文件；回到项目根目录下，控制台输入 `dotnet build --Configuration Release` 构建项目的 C# 部分，接着输入 `cd HelloTriangle/bin/x64/Release/net10.0` 前往构建输出目录，将刚才编译好的 .dll \ .so 文件复制到输出目录下，然后输入 `./HelloTriangle` 即可运行.
