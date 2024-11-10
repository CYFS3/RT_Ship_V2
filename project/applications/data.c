#include <rtthread.h>
#include <rtdevice.h>
#include <cJSON.h>
#include <data.h>
#include "780eg.h"
#include <stdio.h>
#include "control.h"
ship_data ship = {0};

void data_dispose(char * data)
{
	cJSON *root = cJSON_Parse(data);
	if(root != RT_NULL)
	{
		cJSON * w_lv = cJSON_GetObjectItem(root,"w_lv");
		if (w_lv != RT_NULL)
		{
			ship.w_lv = w_lv->valuedouble;
		}
		cJSON * r_lv = cJSON_GetObjectItem(root,"r_lv");
		if (r_lv != RT_NULL)
		{
			ship.r_lv = r_lv->valuedouble;
		}
		cJSON * tur_lv = cJSON_GetObjectItem(root,"tur_lv");
		
		if (tur_lv != RT_NULL)
		{
			ship.tur_lv = tur_lv->valueint;
		}
	}
}


void data_thread_entry(void *parameter)
{
	char buff[512];
	rt_thread_mdelay(1000);
	while (1)
	{
		
		snprintf(buff,sizeof(buff),"{\"order\":1,\"r_lv\":%.2f,\"w_lv\":%.2f,\"tur_lv\":%d,\"w_t\":%.2f,\"tds\":%d}",ship.w_lv,ship.r_lv,ship.tur_lv,ship.w_t,ship.tds);
		send_data(buff);
		rt_thread_mdelay(2000);
	}
	
}

int data_thread_init(void)
{
	rt_thread_t data_thread;
	data_thread = rt_thread_create("data",
									data_thread_entry,
									RT_NULL,
									1024*2,
									RT_THREAD_PRIORITY_MAX / 2,
									20);
	if (data_thread != RT_NULL)
	{
		rt_thread_startup(data_thread);
	}
	return RT_EOK;
}
INIT_APP_EXPORT(data_thread_init);
