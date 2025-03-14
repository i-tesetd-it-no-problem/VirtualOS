/**
 * @file crc.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief CRC校验
 * @version 1.0
 * @date 2024-08-12
 * 
 * @copyright Copyright (c) 2024-2025
 * @see repository: https://github.com/i-tesetd-it-no-problem/VirtualOS.git
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

#ifndef __VIRTUAL_OS_CRC_H__
#define __VIRTUAL_OS_CRC_H__

#define VIRTUAL_OS_CRC_EN

#ifdef VIRTUAL_OS_CRC_EN

#include <stdint.h>

uint16_t crc16_update(uint16_t crc, uint8_t data);
uint16_t crc16_update_bytes(uint16_t crc, uint8_t *data, uint32_t len);

#endif /* VIRTUAL_OS_CRC_EN */

#endif /* __VIRTUAL_OS_CRC_H__ */