/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : led.c
 文件描述 : led相关操作


 函数列表 :
				init_led
				SetLed
 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-10-8
            描述 : 

=========================================================================*/
#include <common.h>

/*=========================================================================
 Function:		int init_led( void )

 Description:		初始化bootloader阶段led状态
 Calls:			_init_led
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0
 Others: 调用各个方案的初始化实现
=========================================================================*/
int init_led( void )
{
	//_init_led();
	return 0;
}

/*=========================================================================
 Function:		void SetLed( led_name led , char state )
 Description:		设置某个led灯的状态
 Calls:			_SetLed
 Data Accessed:
 Data Updated:
 Input:			led:		led灯的名称
 				state:	亮灭
 Output:			无
 Return:			无
 Others: 调用各个方案具体实现
=========================================================================*/
void SetLed( led_name led , char state )
{
	//_SetLed( led , state );
}


