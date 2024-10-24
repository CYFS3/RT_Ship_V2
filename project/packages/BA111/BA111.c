#include <rtthread.h>
#include <rtdevice.h>
#include "BA111.h"
#define DBG_TAG "sensor.ba111"
#define DBG_LVL DBG_INFO  
#include <rtdbg.h>
#define BA111_USART_DEVICE_NAME "uart3"
rt_device_t ba111_serial;
int ba111_usart_device_init(char * ba111_dev_name)
{
    ba111_serial = rt_device_find(ba111_dev_name);
	if(ba111_serial == RT_NULL)
	{
		LOG_E("ba111 usart device  %s find fail!\n",BA111_USART_DEVICE_NAME);
		return - RT_ERROR;
	}
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
	/* 修改串口配置参数 */
	config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
	config.data_bits = DATA_BITS_8;           //数据位 8
	config.stop_bits = STOP_BITS_1;           //停止位 1
	config. rx_bufsz      = 128;                   //修改缓冲区 buff size 为 128
	config.parity    = PARITY_NONE;           //无奇偶校验位
	// /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
	if(rt_device_control(ba111_serial, RT_DEVICE_CTRL_CONFIG, &config) != RT_EOK)
	{
		LOG_E("ba111 config fail!\n");
		return -RT_ERROR;
	}

	/* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
	if(rt_device_open(ba111_serial, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
	{
		LOG_E("ba111 dev open fail\n");
		return -RT_ERROR;
	}
	LOG_I("ba111 find success!\n");
	return RT_EOK;
}

rt_err_t ba111_get_tds(int * tds)
{
	char buf[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
	char send_buf[] = SENSE_COMMAD;
    int ret = rt_device_write(ba111_serial,0,send_buf,6);
	
	rt_kprintf("send data %d\n",ret);
	rt_device_read(ba111_serial,-1,(void*)buf,6);
    for(int i = 0;i < 6;i++)
    {
        rt_kprintf("%d : %2X ",i,buf[i]);
    }
	if(buf[0] == 0xAA && ((char)(buf[0] + buf[1] + buf[2] + buf[3] + buf[4]) == buf[5]))
	{
		rt_kprintf("buf[1] = %X,buf[2] = %X\n",buf[1],buf[2]);
		*tds = (int)((int)buf[1] << 8) + (int)buf[2];
		return RT_EOK;
	}
	LOG_E("ba111 read data fail\n");
	return -RT_ERROR;
}


void ba111_test(void)
{
	ba111_usart_device_init(BA111_USART_DEVICE_NAME);
	int tds = 0;
	if(ba111_get_tds(&tds) == RT_EOK)
	{
		rt_kprintf("\nread tsd %d\n",tds);
	}
	else
	{
		rt_kprintf("read error!\n");
	}
}
MSH_CMD_EXPORT(ba111_test,ba111_test)