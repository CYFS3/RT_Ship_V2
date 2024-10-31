#include <rtthread.h>
#include <rtdevice.h>
#include <cJSON.h>
#define DBG_TAG "lora"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#define UART_DEV_NAME "uart2"
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};


static rt_device_t lora_serial;
/* 用于接收消息的信号量 */
static struct rt_semaphore rx_sem;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
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
	rt_device_open(lora_serial, RT_DEVICE_FLAG_INT_RX);
	rt_device_set_rx_indicate(lora_serial, uart_input);  
    return RT_EOK;
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
	char ch;
	rt_size_t length = 0;
	char buffer[128];
	rt_mailbox_t control_mb = rt_mb_create("control",10,RT_IPC_FLAG_FIFO);
    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(lora_serial, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
		if(ch == '{')
		{
			length = 0;
			buffer[length++] = ch;
		}
		else if(ch == '}')
		{
			buffer[length++] = ch;
			buffer[length] = '\0';
			cJSON *root = cJSON_Parse(buffer);

			if(root != RT_NULL)
			{
				cJSON *item = cJSON_GetObjectItem(root, "order");
				if(item != RT_NULL)
				{
					
					rt_mb_send(control_mb,item->valueint);
					
				}
				cJSON_Delete(root);
			}

		}
		else
		{
			buffer[length++] = ch;
		}
        
    }
}
int lora_thread_init(void)
{
	rt_thread_t lora_thread = RT_NULL;
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);	lora_thread = rt_thread_create("lora",lora_thread_entry,RT_NULL,1024,15,10);
	if(lora_thread != RT_NULL)
	{
		rt_thread_startup(lora_thread);
		return RT_EOK;
	}
	LOG_E("lora thread star fail!\n");
	return -RT_ERROR;
}
INIT_APP_EXPORT(lora_thread_init);
