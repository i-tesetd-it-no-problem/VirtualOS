/**
 * @file qfsm.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 状态机组件
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

#include "utils/qfsm.h"

static const qevent_t q_reserve_ev[] = {
	{ (qsignal_t)Q_EMPTY_SIG },
	{ (qsignal_t)Q_ENTRY_SIG },
	{ (qsignal_t)Q_EXIT_SIG },
	{ (qsignal_t)Q_INIT_SIG },
};

void qfsm_init(qfsm_t *me, qstate_handler f_initial, qevent_t const *e)
{
	me->state = f_initial;
	(*me->state)(me, e);
	(void)(*me->state)(me, &q_reserve_ev[Q_ENTRY_SIG]);
}

void qfsm_dispatch(qfsm_t *const me, qevent_t const *e)
{
	qstate_handler s = me->state;
	qstate r;
	r = (*s)(me, e);

	if (r == Q_EVENT_TRAN) {
		(void)(*s)(me, &q_reserve_ev[Q_EXIT_SIG]);
		(void)(*me->state)(me, &q_reserve_ev[Q_ENTRY_SIG]);
	}
}