# VirtualOS 框架简介(已于2024-12-25日重构)

VirtualOS 是一个基于前后台调度为核心搭建的裸机嵌入式开发框架，其参考了Linux设备驱动的注册方式,将应用与驱动完全分离,并以异步的方式实现了任务调度机制。由于无操作系统,因此称为VirtualOS 

## 框架介绍
- **component**：包含了部分开源组件
- **core**: 框架核心启动代码
- **dal**：设备抽象层，提供通用接口，供应用层调用。
- **docs**：框架文档
- **driver** : 驱动的注册与管理
- **include** : 框架所有头文件
- **Plugin**：插件
- **protocol**: 协议
- **utils**:框架提供的组件
- **toolchain.cmake**: 交叉编译工具链配置文件
- **Virtual.cmake**: 框架的配置文件,所有的项目都需要包含此文件

注：
1. 本框架的编译工程使用免费开源工具链armgcc搭配CMake进行编译、下载和调试（如需在 Keil 上应用，请自行配置.
2. 编译环境配置流程如下（建议将所有下载资源放在一个统一文件夹下管理.

## 推荐工具下载和配置

1. **下载 CMake 工具**  
   [CMake 下载页面](https://cmake.org/download/)  
   选择 cmake-3.30.0-windows-x86_64.msi（或最新版）下载安装，并将 bin 目录加入系统 Path 环境变量。

2. **下载 ARMGCC 交叉编译工具链**  
   [ARMGCC 下载页面](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
   选择 arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-arm-none-eabi.zip（或最新版）下载安装

3. **下载 Ninja**  
   [Ninja 下载页面](https://github.com/ninja-build/ninja/releases/v1.12.1)  
   选择 ninja-win.zip 下载解压，完成后将目录加入系统 Path 环境变量。

4. **下载 J-Link 驱动(或Openocd)**  
   [J-Link 驱动下载页面](https://www.segger.com/downloads/jlink/)  
   选择最新版 Windows 平台下的安装软件。

   [OpenOCD 下载界面](https://gnutoolchains.com/arm-eabi/openocd/)
   选择最新版安装,并将Bin目录加入环境变量

5. **下载 VSCode**  
   [VSCode 下载页面](https://code.visualstudio.com/)  

6. **安装 VSCode 插件**  
   在 VSCode 左侧扩展商店里分别下载以下插件：
   - C/C++（实现语法检查、代码补全、高亮等功能）
   - C/C++ Extension Pack
   - Cortex-Debug（调试代码）

确保 CMake、Ninja 的 bin 目录都已经加入系统的 Path 环境变量中。

## 使用说明
1. [新建工程](./docs/new_project/README.md)
2. [如何使用日志组件](./docs/config_log/README.md)
3. [如何使用按键组件](./docs/config_btn/README.md)