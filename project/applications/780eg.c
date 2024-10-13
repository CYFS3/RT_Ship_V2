#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


void drives_uart_4g_test(void * rt)
{
	rt_device_t dev = rt_device_find("uart4");
	char * re = "hello word!\n";
	rt_device_open(dev, RT_DEVICE_FLAG_INT_RX);
	while (1)
	{
		rt_device_write(dev,0,re,rt_strlen(re));
            rt_thread_mdelay(1000);
	}
	

}
void  test(void)
{
	rt_thread_t test = rt_thread_create("test",drives_uart_4g_test,RT_NULL,1024,10,10);
	rt_thread_startup(test);
}
MSH_CMD_EXPORT(test,drives_uart_4g_test)




