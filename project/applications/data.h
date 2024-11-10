#ifndef __DATA_H__
#define __DATA_H__

typedef struct 
{
	float w_lv;/* 水位 */
	float r_lv;/* 雨滴 */
	int tds;/* TDS值 */
	float ph_val;/* PH值 */
	float w_t;/* 水体温度 */
	char tur_lv;/* 浑浊度等级 */
}ship_data;

void data_dispose(char * data);
#endif /* __DATA_H__ */
