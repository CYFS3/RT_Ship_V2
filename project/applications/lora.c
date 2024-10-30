#include <rtthread.h>
#include <rtdevice.h>
#define DBG_TAG "lora"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#define UART_DEV_NAME "uart4"
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};


static rt_device_t lora_serial;
static rt_mq_t lora_rx_mq = RT_NULL;   

static rt_err_t lora__rx_callback(rt_device_t dev,rt_size_t size)
{
	rt_err_t result;
    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;
    result = rt_mq_send(lora_rx_mq, &msg, sizeof(msg));
    if ( result == -RT_EFULL)
    {
        /* 消息队列满 */
        LOG_D("message queue full!\n");
    }
    return result;
}

int lora_init(void)
{
	lora_serial = rt_device_find(UART_DEV_NAME);
	if(lora_serial == RT_NULL)
	{
		LOG_E("lord dev find fail!\n");
		return -RT_ERROR;
	}
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
	config.baud_rate = BAUD_RATE_115200;        
	config.data_bits = DATA_BITS_8;           
	config.stop_bits = STOP_BITS_1;          
	config.bufsz     = 128;                   
	config.parity    = PARITY_NONE;    
	rt_device_control(lora_serial, RT_DEVICE_CTRL_CONFIG, &config);
	rt_device_open(lora_serial, RT_DEVICE_FLAG_DMA_RX);
	rt_device_set_rx_indicate(lora_serial, lora__rx_callback);   
}
INIT_BOARD_EXPORT(lora_init);
rt_err_t lord_send(char * str)
{
	if(lora_serial == RT_NULL)
	{
		LOG_E("lora dev is null!\n");
		return -RT_ERROR;
	}
	return rt_device_write(lora_serial,0,str,rt_strlen(str));

}
void lora_thread_entry(void * parameter)
{
	struct rx_msg msg;
    rt_ssize_t result;
    rt_uint32_t rx_length;
    static char rx_buffer[128 + 1];

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息*/
        result = rt_mq_recv(lora_rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result > 0)
        {
            /* 从串口读取数据*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rx_buffer[rx_length] = '\0';
            //TODO:manage data change
        }
    }
}
int lora_thread_init(void)
{
	rt_thread_t lora_thread = RT_NULL;
	lora_rx_mq = rt_mq_create("lc29h_rx_mq", sizeof(struct rx_msg), 10, RT_IPC_FLAG_FIFO);
	lora_thread = rt_thread_create("lora",lora_thread_entry,RT_NULL,1024,15,10);
	if(lora_thread != RT_NULL)
	{
		rt_thread_startup(lora_thread);
		return RT_EOK;
	}
	LOG_E("lora thread star fail!\n");
	return -RT_ERROR;
}
INIT_APP_EXPORT(lora_thread_init);
