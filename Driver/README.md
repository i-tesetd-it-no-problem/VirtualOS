# 驱动层 (Driver)

Driver层用于实现特定平台的读写等芯片操作,所有字符设备驱动通过device_register接口进行注册，并通过sys.h文件中的EXPORT_DIRVER宏导出

## arm_core 
 - CMSIS相关的头文件 不需要动
  
## system 
  - internal_commands.c
    - 系统内部命令 
  - system/platform_schedule.c
    - 用户实现部分内容,用于调度任务
  - sys.h
    - 系统头文件,提供驱动注册宏，所有的驱动都需要使用此宏进行注册
  - syscall.c
    - 绕过编译器警告错误,裸机中无意义	
  - syslog.c
     - 如需使用dal_log日志接口,需要实现部分接口,不使用则不需要动
  - virtualos_run.c
    - 框架启动入口,初始化框架资源,创建日志任务等

## 驱动编写模板
``` C
#include "sys.h" /* 必要的头文件 */
#include "device.h" /* 必要的头文件 */

/* 导入芯片头文件 */

static const char xxx_name[] = "xxx"; /* 确保此设备名唯一 */
static drv_err_e xxx_open(drv_file_t *file);
static drv_err_e xxx_close(drv_file_t *file);
static drv_err_e xxx_ioctrl(drv_file_t *file, int cmd, void *arg);
static int xxx_read(drv_file_t *file, uint8_t *buf, size_t len);
static int xxx_write(drv_file_t *file, const uint8_t *buf, size_t len);
static int xxx_lseek(drv_file_t *file, int offset, drv_lseek_whence_e whence);

static drv_err_e xxx_open(drv_file_t *file)
{
	if (file->is_opened)
		return DRV_ERR_OCCUPIED;

	file->is_opened = true;

	/* 外设初始化 */

	file->dev_size = 0; /* 设备大小 可选 存储类设备需要 */

	return DRV_ERR_NONE;
}

static drv_err_e xxx_close(drv_file_t *file)
{
	/* 关闭外设 例如进入低功耗 */

	file->is_opened = false;

	return DRV_ERR_NONE;
}

static drv_err_e xxx_ioctrl(drv_file_t *file, int cmd, void *arg)
{
	if (!file->is_opened)
		return DRV_ERR_UNAVALIABLE;

	/* 除读写外的控制命令处理 */

	return DRV_ERR_NONE;
}

static int xxx_read(drv_file_t *file, uint8_t *buf, size_t len)
{
	if (!file->is_opened)
		return DRV_ERR_UNAVALIABLE;

	/* 读取外设数据 */

	return 0; /* 读取成功返回实际读取的字节数 */
}

static int xxx_write(drv_file_t *file, const uint8_t *buf, size_t len)
{
	if (!file->is_opened)
		return DRV_ERR_UNAVALIABLE;

	/* 写入外设数据 */

	return 0; /* 写入成功返回实际写入的字节数 */
}

static int xxx_lseek(drv_file_t *file, int offset, drv_lseek_whence_e whence)
{
	uint32_t cur_offset = file->seek_pos;
	uint32_t new_offset = 0;

	switch (whence) {
	case DRV_LSEEK_WHENCE_HEAD: /* 头 */
		new_offset = offset;
		break;

	case DRV_LSEEK_WHENCE_SET: /* 当前 */
		new_offset = cur_offset + offset;
		break;

	case DRV_LSEEK_WHENCE_TAIL: /* 尾 */
		new_offset = file->dev_size - 1 + offset;
		break;

	default:
		return DRV_ERR_INVALID; /* 无效的whence参数 */
	}

	if (new_offset >= file->dev_size || new_offset < 0)
		return DRV_ERR_INVALID; /* 偏移超出有效范围 */

	file->seek_pos = new_offset;

	return new_offset;
}

static drv_file_opts_t xxx_dev = {
	.close = xxx_close,
	.ioctrl = xxx_ioctrl,
	.lseek = xxx_lseek,
	.open = xxx_open,
	.read = xxx_read,
   	.write = xxx_write,
};

void xxx_init(void)
{
   device_register(&xxx_dev, xxx_name);
}

EXPORT_DIRVER(xxx_init) //必须使用EXPORT_DIRVER宏导出驱动初始化函数
```

## 许可证

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.