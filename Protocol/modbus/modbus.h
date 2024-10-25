#ifndef _MODBUS_H
#define _MODBUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MODBUS_FUN_RD_REG_MUL (0x03) // 读功能码
#define MODBUS_FUN_WR_REG_MUL (0x10) // 写功能码

#define MODBUS_REG_NUM_MAX (126) // 最大寄存器数量

#define MODBUS_ADDR_BYTES_NUM (1)	 // 地址字节数
#define MODBUS_FUNC_BYTES_NUM (1)	 // 功能码字节数
#define MODBUS_REG_BYTES_NUM (2)	 // 寄存器地址字节数
#define MODBUS_REG_LEN_BYTES_NUM (2) // 寄存器长度字节数
#define MODBUS_CRC_BYTES_NUM (2)	 // CRC字节数

// 一帧最大字节数 256
#define MODBUS_FRAME_BYTES_MAX (256)

// 校验功能码
#define MODBUS_FUNC_CHECK_VALID(f)                                                                 \
	(((f) == MODBUS_FUN_RD_REG_MUL) || ((f) == MODBUS_FUN_WR_REG_MUL))

// 校验寄存器范围
#define MODBUS_CHECK_REG_RANGE(reg, num, from, to)                                                 \
	(((reg) <= (to)) && ((reg) >= (from)) && (num <= MODBUS_REG_NUM_MAX) &&                        \
		(((reg) + (num) - 1)) <= (to))

#define COMBINE_U8_TO_U16(h, l) ((uint16_t)(h << 8) | (uint16_t)(l))
#define COMBINE_U16_TO_U32(h, l) ((uint32_t)(h << 16) | (uint32_t)(l))
#define GET_U8_HIGH_FROM_U16(u) (((u) >> 8) & 0xff)
#define GET_U8_LOW_FROM_U16(u) ((u) & 0xff)

// 串口方向
enum modbus_serial_dir {
	modbus_serial_dir_all_unuse,
	modbus_serial_dir_rx_only,
	modbus_serial_dir_tx_only,
};

/**
 * @brief 串口初始化函数指针
 *
 * @return true 初始化成功
 * @return false 初始化失败
 */
typedef bool (*modbus_serial_init)(void);

/**
 * @brief 串口写函数指针
 *
 * @param p_data 待写入数据指针
 * @param len 待写入数据长度
 * @return size_t 实际写入数据长度 失败返回0
 */
typedef size_t (*modbus_serial_write)(uint8_t *p_data, uint16_t len);

/**
 * @brief 串口读函数指针
 *
 * @param p_data 读入数据指针
 * @param len 待读数据长度
 * @return size_t 实际读入数据长度 失败返回0
 */
typedef size_t (*modbus_serial_read)(uint8_t *p_data, uint16_t len);

/**
 * @brief 串口方向控制函数指针
 *
 * @param ctrl 串口方向控制
 */
typedef void (*modbus_serial_dir_ctrl)(enum modbus_serial_dir ctrl);

/**
 * @brief 检查是否发送完成
 * 
 * 如果是轮训发送可直接设置为NULL
 * DAM发送则需要查询相关标志位
 * 
 * @return ture:发送完/可以接收 false:发送中
 */
typedef bool (*modbus_serial_check_send)(void);

// 串口回调
struct serial_opts {
	modbus_serial_init f_init;			   // 串口初始化函数指针
	modbus_serial_write f_write;		   // 串口写函数指针
	modbus_serial_read f_read;			   // 串口读函数指针
	modbus_serial_dir_ctrl f_dir_ctrl;	   // 串口方向控制函数指针
	modbus_serial_check_send f_check_send; // 判断是否发送完成
};

#endif