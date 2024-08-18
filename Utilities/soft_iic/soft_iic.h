/**
 * @file soft_iic.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 软件IIC组件
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

#ifndef _VIRTUAL_OS_SOFT_IIC_H
#define _VIRTUAL_OS_SOFT_IIC_H

#include <stdint.h>

typedef enum {
	iic_low = 0,
	iic_high = !iic_low,
} soft_iic_level_e;

typedef void (*soft_iic_delay_f)(uint8_t m_us);
typedef void (*soft_iic_scl_out_f)(soft_iic_level_e level);
typedef void (*soft_iic_sda_out_f)(soft_iic_level_e level);
typedef uint8_t (*soft_iic_sda_in_f)(void);

uint8_t soft_iic_write_one_byte(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t soft_iic_write_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
uint8_t soft_iic_read_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

/**
 * 软件iic初始化，成功返回1，失败返回0
 * @param scl_out_f scl输出函数
 * @param sda_out_f sda输出函数
 * @param sda_in_f sda读取函数
 * @param delay_f  us级延时函数,没有设置为NULL
 *
 * @return 0 or 1
*/
uint8_t soft_iic_init(soft_iic_scl_out_f scl_out_f, soft_iic_sda_out_f sda_out_f, soft_iic_sda_in_f sda_in_f, soft_iic_delay_f delay_f);
#endif /*_VIRTUAL_OS_SOFT_IIC_H*/
