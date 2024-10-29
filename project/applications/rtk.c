#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


#ifdef BSP_USING_LC29H
#include "gps_rmc.h"
#define DBG_TAG "lc29h"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};
rt_device_t lc29h_dev = RT_NULL;
rt_mq_t lc29h_rx_mq = RT_NULL;

   
/* 接收数据回调函数 */
static rt_err_t lc29h_rx_callback(rt_device_t dev, rt_size_t size)
{
    
 	rt_err_t result;
    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;
    result = rt_mq_send(lc29h_rx_mq, &msg, sizeof(msg));
    if ( result == -RT_EFULL)
    {
        /* 消息队列满 */
        LOG_D("message queue full!\n");
    }
    return result;
}

int lc29h_init(void)
{
	lc29h_dev = rt_device_find(BSP_LC29H_UART);
	if (lc29h_dev == RT_NULL)
	{
		return -RT_ERROR;
	}
	lc29h_rx_mq = rt_mq_create("lc29h_rx_mq", sizeof(struct rx_msg), 10, RT_IPC_FLAG_FIFO);
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

	config.baud_rate = BAUD_RATE_115200;        
	config.data_bits = DATA_BITS_8;           
	config.stop_bits = STOP_BITS_1;          
	config.bufsz      = 1024*2;                   
	config.parity    = PARITY_NONE;          

	rt_device_control(lc29h_dev, RT_DEVICE_CTRL_CONFIG, &config);
	rt_device_open(lc29h_dev, RT_DEVICE_FLAG_DMA_RX);
	rt_device_set_rx_indicate(lc29h_dev, lc29h_rx_callback);
	
	

}
INIT_BOARD_EXPORT(lc29h_init);
static void serial_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_ssize_t result;
    rt_uint32_t rx_length;
    static char rx_buffer[BSP_LC29H_RB_BUSIZ + 1];
    struct gps_info info_data = {0};
    gps_info_t info = &info_data;
    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息*/
        result = rt_mq_recv(lc29h_rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result > 0)
        {
            /* 从串口读取数据*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rx_buffer[rx_length] = '\0';
            char *p = strstr(rx_buffer, "$GNRMC");
			if(p != RT_NULL)
            {
                char *q = strstr(p, "\r\n");
                if(q != RT_NULL)
                {
                    q += 2;
                    *q = '\0';
                    rt_kprintf("rx_buffer:%s\n",p);
                    if (gps_rmc_parse(info, p))
                    {
                        //gps_print_info(info);
                    }
                    rt_memset(info, 0, sizeof(struct gps_info));
                }
            }
        }
    }
}
int lc29h_thread_init(void)
{
	rt_thread_t thread = RT_NULL;
	thread = rt_thread_create("lc29h_thread", serial_thread_entry, RT_NULL, 2048, 1, 10);
	if (thread != RT_NULL)
	{
		rt_thread_startup(thread);
	}
	return 0;
}
INIT_APP_EXPORT(lc29h_thread_init);
#endif /* BSP_USING_LC29H */




