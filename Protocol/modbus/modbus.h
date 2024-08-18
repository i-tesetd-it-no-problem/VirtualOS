/**
 * @file modbus.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief ModBus协议
 * @version 1.0
 * @date 2024-08-12
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#ifndef _VIRTUAL_OS_MODBUS_H
#define _VIRTUAL_OS_MODBUS_H

#include <stdint.h>

#define MODBUS_FUN_RD_REG_MUL (0x03)
#define MODBUS_FUN_WR_REG_MUL (0x10)
#define MODBUS_FUN_REG_USER (0x5A)

#define MODBUS_REG_NUM_MAX (130)

#define MODBUS_FRAME_BYTES_MAX (512)

#define MODBUS_CRC_BYTES_NUM (2)

#define MODBUS_DATA_BYTES_MAX ((MODBUS_REG_NUM_MAX << 1) + MODBUS_CRC_BYTES_NUM)

#define MODBUS_FUNC_CHECK_VALID(f) (((f) == MODBUS_FUN_RD_REG_MUL) || ((f) == MODBUS_FUN_WR_REG_MUL) || ((f) == MODBUS_FUN_REG_USER))

#define MODBUS_CHECK_REG_RANGE(reg, num, from, to) (((reg) <= (to)) && ((reg) >= (from)) && (((reg) + (num) - 1)) <= (to))

#define COMBINE_U8_TO_U16(h, l) ((uint16_t)(h << 8) | (uint16_t)(l))
#define COMBINE_U16_TO_U32(h, l) ((uint32_t)(h << 16) | (uint32_t)(l))
#define GET_U8_HIGH_FROM_U16(u) (((u) >> 8) & 0xff)
#define GET_U8_LOW_FROM_U16(u) ((u) & 0xff)

typedef enum {
	modbus_serial_dir_all_unuse,
	modbus_serial_dir_rx_only,
	modbus_serial_dir_tx_only,
} MODBUS_SERIAL_DIR_E;

typedef int (*rtu_address_validator)(uint16_t address);
typedef int (*modbus_serial_init)();
typedef int (*modbus_serial_write)(uint8_t *p_data, uint16_t len);
typedef int (*modbus_serial_read)(uint8_t *p_data, uint16_t len);
typedef void (*modbus_serial_flush)(void);
typedef void (*modbus_serial_dir_ctrl)(MODBUS_SERIAL_DIR_E ctrl);

typedef struct {
	modbus_serial_init f_init;
	modbus_serial_write f_write;
	modbus_serial_read f_read;
	modbus_serial_flush f_flush;
	modbus_serial_dir_ctrl f_dir_ctrl;
} modbus_serial_opt_t;

#endif /*_VIRTUAL_OS_MODBUS_H*/