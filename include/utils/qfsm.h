/**
 * @file qfsm.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 状态机组件
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

#ifndef _VIRTUAL_OS_QFSM_H
#define _VIRTUAL_OS_QFSM_H

#include <stdint.h>

typedef uint32_t qsignal_t;
typedef int qstate;

typedef struct _qeventtag {
	qsignal_t sig;
} qevent_t;

typedef struct qfsmtag qfsm_t;

typedef qstate (*qstate_handler)(qfsm_t *me, qevent_t const *e);

struct qfsmtag {
	qstate_handler state;
};

enum q_event_result {
	Q_EVENT_HANDLED = (qstate)0,
	Q_EVENT_IGNORED,
	Q_EVENT_TRAN,
};

#define Q_HANDLED() (Q_EVENT_HANDLED)
#define Q_IGNORED() (Q_EVENT_IGNORED)
#define Q_TRAN(target_) (((qfsm_t *)me)->state = (qstate_handler)(target_), Q_EVENT_TRAN)

enum q_reserve_signal {
	Q_EMPTY_SIG = (qsignal_t)0,
	Q_ENTRY_SIG,
	Q_EXIT_SIG,
	Q_INIT_SIG,

	/*自定义信号区域*/
	Q_APP_EVENT_TIMEOUT,
};

/**
 * @brief 状态机初始化
 * 
 * @param me 状态机实例
 * @param f_initial 初始化状态
 * @param e 事件
 */
void qfsm_init(qfsm_t *me, qstate_handler f_initial, qevent_t const *e);

/**
 * @brief 状态机调度
 * 
 * @param me 状态机实例
 * @param e 事件
 */
void qfsm_dispatch(qfsm_t * const me, qevent_t const *e);
#endif /*_VIRTUAL_OS_QFSM_H*/