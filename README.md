# VirtualOS 框架简介

VirtualOS 是一个基于前后台调度为核心搭建的裸机嵌入式开发框架，其参考了Linux设备驱动的注册方式,将应用与驱动完全分离,并以异步的方式实现了任务调度机制。由于无操作系统,因此称为VirtualOS 

## 框架介绍
- **Component**：包含了部分开源组件
- **DAL**：设备抽象层，提供通用接口，供应用层调用。
- **Docs**：框架文档
- **Driver** : 与具体硬件平台相关的驱动
- **Plugin**：插件
- **Project**：实际的工程项目
- **Protocol**: 协议层
- **Utilities**:框架提供的组件
- **VFS** : 虚拟文件系统层,设备注册相关接口(非Linux的VFS)
- **toolchain.cmake**: 工具链配置文件
- **Virtual.cmake**: 框架的顶层工程配置文件,所有的项目都需要包含此文件

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

## 新建工程请参考 [Docs/NewProject/README.md](Docs/NewProject/README.md)

## 如需使用日志功能请参考 [Docs/ConfigSyslog/README.md](Docs/ConfigSyslog/README.md)