## 使用框架自带的日志功能需要实现Driver/ststem/syslog.c中的三个接口
 - log_device_init - 外设的初始化
 - log_device_transmit - 外设的发送函数
 - log_device_recieve - 外设的接收函数
 - 注意 : 在判断完发送结束以后一定要调用free_log();

### 方案一 : 使用JLink驱动自带的打印驱动
    以下代码仅供参考
```C
#include "SEGGER_RTT.h" /* 导入驱动头文件(框架已提供,直接导入即可) */

/* 请勿修改接口定义 */
static void log_device_init(void)
{
	/* 完成日志所用设备的初始化操作,例如串口的初始化 */

    /* 空函数 Jlink打印不需要初始化 */
}

static int log_device_transmit(uint8_t *buf, size_t len)
{
	/* 完成发送接口,例如串口发送,SEGGER_RTT_Write等 */

    SEGGER_RTT_Write(0, buf, len);
    free_log(); /* 每一次发送完成都需要调用此接口 */

	return len; /* 返回发送成功的字节数 */
}

static int log_device_recieve(uint8_t *buf, size_t len)
{
	/* 完成接收接口,例如串口接收,SEGGER_RTT_Read等 */

	return SEGGER_RTT_Read(0, buf, len); /* 返回实际接收的字节长度 */
}
```

### 方案二 : 使用芯片串口作为日志收发(以GD32F30X系列为例)
    强烈建议使用 串口 + DMA + 超时中断(或空闲中断) 作为日志的收发配置

    以下代码仅供参考

```C
#include "gd32f30x.h" /* 芯片头文件 */

static uint8_t tx_buf[MAX_LOG_LENGTH]; /* 发送缓冲 */
static uint8_t rx_buf[MAX_LOG_LENGTH]; /* 接收缓冲 */

static struct queue_info rx_q; /* 接收队列 队列组件框架已提供 */

/* 请勿修改接口定义 */
static void log_device_init(void)
{
	/* 完成日志所用设备的初始化操作,例如串口的初始化 */

	rcu_periph_clock_enable(RCU_GPIOA); /* 使能GPIO时钟 */
	gpio_init(GPIOA, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_9); /* TX */
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10); /* RX */

	rcu_periph_clock_enable(RCU_USART0); /* 使能串口时钟 */
	usart_baudrate_set(USART0, 115200); /* 波特率115200 ,其他停止位等使用默认的无需额外配置 */
	usart_receive_config(USART0, USART_RECEIVE_ENABLE); /* 使能接收 */
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE); /* 使能发送 */
	usart_interrupt_enable(USART0, USART_INT_RT); /* 超时中断(空闲中断也可以) */
	usart_receiver_timeout_enable(USART0); /* 使能接收超时 */
	usart_receiver_timeout_threshold_config(USART0, 10); /* 超时阈值设置10 */
	nvic_irq_enable(USART0_IRQn, 0, 0); /* 配置中断优先级 */
	usart_enable(USART0); /* 使能串口 */

	rcu_periph_clock_enable(RCU_DMA0); /* 使能DMA时钟 */
	dma_deinit(DMA0, DMA_CH3); /* 复位 */
	dma_parameter_struct dma_param; /* DMA参数 */
	dma_struct_para_init(&dma_param); /* 初始化为默认参数 */
	dma_param.periph_addr = (uint32_t)(&USART_DATA(USART0)); /* 串口0数据寄存器地址 */
	dma_param.periph_width = DMA_PERIPHERAL_WIDTH_8BIT; /* 8位宽 */
	dma_param.memory_addr = (uint32_t)tx_buf; /* 内存地址 发送缓冲 */
	dma_param.memory_width = DMA_MEMORY_WIDTH_8BIT; /* 8位宽 */
	dma_param.number = MAX_LOG_LENGTH; /* DMA大小 */
	dma_param.priority = DMA_PRIORITY_ULTRA_HIGH; /* DMA优先级 */
	dma_param.periph_inc = DMA_PERIPH_INCREASE_DISABLE; /* 外设地址不自增 */
	dma_param.memory_inc = DMA_MEMORY_INCREASE_ENABLE; /* 内存地址自增 */
	dma_param.direction = DMA_MEMORY_TO_PERIPHERAL; /* 内存到外设 */
	usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE); /* 开启串口发送DMA */
	dma_init(DMA0, DMA_CH3, &dma_param); /* 初始化DMA */
	dma_interrupt_enable(DMA0, DMA_CH3, DMA_INT_FLAG_FTF); /* 发送完成中断 */
	nvic_irq_enable(DMA0_Channel3_IRQn, 0, 1); /* 中断优先级 */

	dma_param.memory_addr = (uint32_t)rx_buf; /* 内存地址 接收缓存 */
	dma_param.direction = DMA_PERIPHERAL_TO_MEMORY; /* 外设到内存 */
	dma_deinit(DMA0, DMA_CH4); /* 复位 */
	dma_init(DMA0, DMA_CH4, &dma_param); /* 初始化 */
	dma_circulation_enable(DMA0, DMA_CH4); /* 使能唤醒缓冲 */
	queue_init(&rx_q, 1, rx_buf, MAX_LOG_LENGTH); /* 初始化队列 */
	usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE); /* 开启串口接收DMA */
	dma_channel_enable(DMA0, DMA_CH4); /* 启动接收DMA */
}

/* 此处可以自行实现平台对应中断处理函数,记住无论使用何种方式判断发送完成,最后都要调用 free_log() */
void DMA0_Channel3_IRQHandler(void)
{
	if (dma_interrupt_flag_get(DMA0, DMA_CH3, DMA_INT_FLAG_FTF) != RESET) {
		free_log(); /* 每一次发送完成都需要调用此接口 */

		dma_interrupt_flag_clear(DMA0, DMA_CH3, DMA_INT_FLAG_FTF);
	}
}

static uint16_t pre_pos = 0; /* DMA上一次指针 */
void USART0_IRQHandler(void)
{
	if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RT) != RESET) {
		uint16_t cur_pos = MAX_LOG_LENGTH - dma_transfer_number_get(DMA0, DMA_CH4); /* DMA当前指针 */
		uint16_t cur_recieve = (cur_pos - pre_pos) & (MAX_LOG_LENGTH - 1); /* 计算接收长度 */
		queue_advance_wr(&rx_q, cur_recieve); /* 移动队列写指针 */
		pre_pos = cur_pos; /* 更新索引 */

		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RT);
	}
}

/* 请勿修改接口定义 */
static int log_device_transmit(uint8_t *buf, size_t len)
{
    /* 完成发送接口,例如串口发送,SEGGER_RTT_Write等 */
    memcpy(tx_buf, buf, len); /* 复制到发送缓冲 */
    dma_channel_disable(DMA0, DMA_CH3); /* 关闭DMA */
    dma_transfer_number_config(DMA0, DMA_CH3, len); /* 配置发送长度 */
    dma_channel_enable(DMA0, DMA_CH3); /* 开启DMA */
    return len;
}

/* 请勿修改接口定义 */
static int log_device_recieve(uint8_t *buf, size_t len)
{
	/* 完成接收接口,例如串口接收,SEGGER_RTT_Read等 */

	return queue_get(&rx_q, buf, len); /* 从队列取数据,返回实际取到的数据长度 */
}
```

### 现已去除大部分默认命令,如需使用可在VFS/vfs.c文件底部取消注释相关代码或driver/system/internal_commands.c中的echo命令

### 默认关闭日志时间显示