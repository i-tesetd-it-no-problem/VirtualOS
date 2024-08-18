/**
 * @file soft_iic.c
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

#include "soft_iic.h"

soft_iic_delay_f soft_iic_delay_us = 0;
soft_iic_scl_out_f scl_out = 0;
soft_iic_sda_out_f sda_out = 0;
soft_iic_sda_in_f sda_in = 0;

void iic_delay(uint8_t us)
{
	if (soft_iic_delay_us)
		soft_iic_delay_us(us);
	else {
		uint8_t i = 7;

		while (i)
			i--;
	}
}

uint8_t soft_iic_init(soft_iic_scl_out_f scl_out_f, soft_iic_sda_out_f sda_out_f, soft_iic_sda_in_f sda_in_f, soft_iic_delay_f delay_f)
{
	if (!(scl_out_f && sda_out_f && sda_in_f))
		return 0;

	scl_out = scl_out_f;
	sda_out = sda_out_f;
	sda_in = sda_in_f;
	soft_iic_delay_us = delay_f;
	return 1;
}

void soft_iic_start(void)
{
	sda_out(iic_high);
	scl_out(iic_high);
	iic_delay(2);
	sda_out(iic_low);
	iic_delay(2);
	scl_out(iic_low);
}

void soft_iic_stop(void)
{
	scl_out(iic_low);
	sda_out(iic_low);
	iic_delay(2);
	scl_out(iic_high);
	sda_out(iic_high);
	iic_delay(2);
}

uint8_t soft_iic_wait_ack(void)
{
	uint8_t ucErrTime = 0;
	sda_out(iic_high);
	iic_delay(2);
	scl_out(iic_high);
	iic_delay(2);

	while (sda_in()) {
		ucErrTime++;

		if (ucErrTime > 250) {
			soft_iic_stop();
			return 1;
		}
	}

	scl_out(iic_low);
	return 0;
}

void soft_iic_ack(void)
{
	scl_out(iic_low);
	sda_out(iic_low);
	iic_delay(2);
	scl_out(iic_high);
	iic_delay(2);
	scl_out(iic_low);
}

void soft_iic_nack(void)
{
	scl_out(iic_low);
	sda_out(iic_high);
	iic_delay(2);
	scl_out(iic_high);
	iic_delay(2);
	scl_out(iic_low);
}

void soft_iic_send_byte(uint8_t data)
{
	uint8_t t;
	scl_out(iic_low);

	for (t = 0; t < 8; t++) {
		sda_out((data & 0x80) >> 7);
		data <<= 1;
		scl_out(iic_high);
		iic_delay(2);
		scl_out(iic_low);
		iic_delay(2);
	}
}

uint8_t soft_iic_rcv_byte(unsigned char ack)
{
	unsigned char i, receive = 0;

	for (i = 0; i < 8; i++) {
		scl_out(iic_low);
		iic_delay(2);
		scl_out(iic_high);
		receive <<= 1;

		if (sda_in())
			receive++;

		iic_delay(2);
	}

	if (!ack)
		soft_iic_nack();
	else
		soft_iic_ack();

	return receive;
}

uint8_t soft_iic_write_one_byte(uint8_t addr, uint8_t reg, uint8_t data)
{
	soft_iic_start();
	soft_iic_send_byte((addr << 1) | 0);

	if (soft_iic_wait_ack()) {
		soft_iic_stop();
		return 1;
	}

	soft_iic_send_byte(reg);
	soft_iic_wait_ack();
	soft_iic_send_byte(data);

	if (soft_iic_wait_ack()) {
		soft_iic_stop();
		return 1;
	}

	soft_iic_stop();
	return 0;
}

uint8_t soft_iic_write_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	uint8_t i;
	soft_iic_start();
	soft_iic_send_byte((addr << 1) | 0);

	if (soft_iic_wait_ack()) {
		soft_iic_stop();
		return 1;
	}

	soft_iic_send_byte(reg);
	soft_iic_wait_ack();

	for (i = 0; i < len; i++) {
		soft_iic_send_byte(buf[i]);

		if (soft_iic_wait_ack()) {
			soft_iic_stop();
			return 1;
		}
	}

	soft_iic_stop();
	return 0;
}

uint8_t soft_iic_read_bytes(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	soft_iic_start();
	soft_iic_send_byte((addr << 1) | 0);

	if (soft_iic_wait_ack()) {
		soft_iic_stop();
		return 1;
	}

	soft_iic_send_byte(reg);
	soft_iic_wait_ack();
	soft_iic_start();
	soft_iic_send_byte((addr << 1) | 1);
	soft_iic_wait_ack();

	while (len) {
		if (len == 1)
			*buf = soft_iic_rcv_byte(0);
		else
			*buf = soft_iic_rcv_byte(1);

		len--;
		buf++;
	}

	soft_iic_stop();
	return 0;
}
