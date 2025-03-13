# 驱动编写模板

```c
#include <stdbool.h>
#include "driver/virtual_os_driver.h" /* 驱动注册头文件 */

static const char xxx_name[] = "xxx"; /* 确保此设备名项目中唯一 */
static int xxx_open(struct drv_file *file);
static int xxx_close(struct drv_file *file);
static int xxx_ioctl(struct drv_file *file, int cmd, void *arg);
static size_t xxx_read(struct drv_file *file, void *buf, size_t len, size_t *offset);
static size_t xxx_write(struct drv_file *file, void *buf, size_t len, size_t *offset);

static int xxx_open(struct drv_file *file)
{
	if (file->is_opened)
		return DRV_ERR_OCCUPIED;

	/* 打开外设 */

	file->is_opened = true;

	return DRV_ERR_NONE;
}

static int xxx_close(struct drv_file *file)
{
	/* 关闭外设 例如进入低功耗 */

	file->is_opened = false;

	return DRV_ERR_NONE;
}

static int xxx_ioctl(struct drv_file *file, int cmd, void *arg)
{
	if (!file->is_opened)
		return DRV_ERR_UNAVAILABLE;

	/* 除读写外的控制命令处理 */

	return DRV_ERR_NONE;
}

static size_t xxx_read(struct drv_file *file, void *buf, size_t len, size_t *offset)
{
	if (!file->is_opened)
		return 0;

	/* 读取外设数据 */

	return 0; /* 读取成功返回实际读取的字节数 */
}

static size_t xxx_write(struct drv_file *file, void *buf, size_t len, size_t *offset)
{
	if (!file->is_opened)
		return 0;

	return 0; /* 写入成功返回实际写入的字节数 */
}

// 设备操作接口
static const struct file_operations xxx_opts = {
	.close = xxx_close,
	.ioctl = xxx_ioctl,
	.open = xxx_open,
	.read = xxx_read,
	.write = xxx_write,
};

// 设备驱动初始化
static bool xxx_driver_init(struct drv_device *dev)
{
	/* 外设初始化 */

	return true;
}

// 通过宏定义导出驱动(在 virtual_os_init 函数中会被调用)
EXPORT_DRIVER(xxx_driver_probe)
void xxx_driver_probe(void)
{
	driver_register(xxx_driver_init, &xxx_opts, xxx_name); // 调用注册接口
}
```
