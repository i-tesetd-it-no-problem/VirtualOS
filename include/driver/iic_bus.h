#ifndef __VIRTUAL_OS_IIC_BUS_H__
#define __VIRTUAL_OS_IIC_BUS_H__

#include "stddef.h"
#include <stdint.h>

#define I2C_FLAG_WRITE (0x00) // 写标志
#define I2C_FLAG_READ (0x01)  // 读标志

struct i2c_msg {
	size_t len;	   // 数据长度
	uint8_t *buf;  // 数据指针
	uint8_t addr;  // 只支持7位地址
	uint8_t flags; // 读写标志
};

#endif /* __VIRTUAL_OS_IIC_BUS_H__ */