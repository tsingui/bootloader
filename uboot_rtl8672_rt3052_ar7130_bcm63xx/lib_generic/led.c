/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : led.c
 �ļ����� : led��ز���


 �����б� :
				init_led
				SetLed
 �޶���¼ :
          1 ���� : ������
            ���� : 2008-10-8
            ���� : 

=========================================================================*/
#include <common.h>

/*=========================================================================
 Function:		int init_led( void )

 Description:		��ʼ��bootloader�׶�led״̬
 Calls:			_init_led
 Data Accessed:
 Data Updated:
 Input:			��
 Output:			��
 Return:			0
 Others: ���ø��������ĳ�ʼ��ʵ��
=========================================================================*/
int init_led( void )
{
	//_init_led();
	return 0;
}

/*=========================================================================
 Function:		void SetLed( led_name led , char state )
 Description:		����ĳ��led�Ƶ�״̬
 Calls:			_SetLed
 Data Accessed:
 Data Updated:
 Input:			led:		led�Ƶ�����
 				state:	����
 Output:			��
 Return:			��
 Others: ���ø�����������ʵ��
=========================================================================*/
void SetLed( led_name led , char state )
{
	//_SetLed( led , state );
}


