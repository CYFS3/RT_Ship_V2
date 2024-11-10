#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "lora"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#define UART_DEV_NAME "uart4"




static rt_device_t lora_serial;
static struct rt_semaphore rx_sem;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

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
			data_dispose(buffer);
		}
		else
		{
			buffer[length++] = ch;
		}
        
    }
}
int lora_thread_init(void)
{
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
	/* step1：查找串口设备 */
	lora_serial = rt_device_find(UART_DEV_NAME);
	/* 查找系统中的串口设备 */
    if (!lora_serial)
    {
        rt_kprintf("find %s failed!\n", UART_DEV_NAME);
        return -RT_ERROR;
    }
	/* step2：修改串口配置参数 */
	config.baud_rate = BAUD_RATE_115200;        //修改波特率为 9600
	config.data_bits = DATA_BITS_8;           //数据位 8
	config.stop_bits = STOP_BITS_1;           //停止位 1
	config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
	config.parity    = PARITY_NONE;           //无奇偶校验位

	/* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
	rt_device_control(lora_serial, RT_DEVICE_CTRL_CONFIG, &config);
    
    /* 初始化信号量 */
    rt_sem_init(&rx_sem, "lora_sem", 0, RT_IPC_FLAG_FIFO);
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(lora_serial, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(lora_serial, uart_input);
    /* 创建 lora_serial 线程 */
    rt_thread_t thread = rt_thread_create("lora_serial", lora_thread_entry, RT_NULL, 1024, 25, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(lora_thread_init);
