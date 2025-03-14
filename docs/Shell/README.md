# 使能Shell

本框架可以通过`include/core/virtual_os_config.h` 中的宏`VIRTUALOS_SHELL_ENABLE`来启动Shell功能。
注意，由于Shell依赖一个具体的芯片平台串口，因此静态库的编译方式不适合启动Sheel功能。因为需要额外引入芯片平台的串口驱动。
因此，如要开启此功能，建议导入`virtual_os_src.cmake`,将VirtualOS的源码全部加入通应用工程一起编译。

如果用户需要在静态库编译方式下使用Shell功能，应该在应用层自行调用`utils/simple_shell`的组件进行初始化，读写回调可使用`dal/optes`中的读写来注册

如果使用源码编译方式只需填充`core/virtual_os_sh_drv.c`中的相关接口即可。

关键代码如下：

```c

#include "core/virtual_os_config.h"

// VIRTUALOS_SHELL_ENABLE // 注意使能此宏

/**
 * @brief 初始化平台相关的串口
 * 
 */
static void platform_serial_init(void)
{
	/* 平台相关的串口初始化 */
}

/**
 * @brief 平台相关的串口的接收回调
 * 
 */
static struct sp_shell_opts sh_opts = {
	.read = NULL,  // 串口读
	.write = NULL, // 串口写
};
```

用户只需要初始化芯片平台的串口，并提供相关的读写回调接口即可使用Shell功能。
