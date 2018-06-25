/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 15/02/13 2:19p $
 * @brief    A project template for Mini58 MCU.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "main.h"

uint8_t Complete_Buff[6] = {0xFF, 0xFF, 0xaa, 0x00, 0x01, 0x3E};
uint8_t NFCModeDetectOK_Buff[7] = {0xFF, 0xFF, 0x07, 0x01, 0x00, 0x06, 0x3E};
uint8_t NFCModeDetectFaild_Buff[7] = {0xFF, 0xFF, 0x07, 0x01, 0x01, 0x07, 0x3E};

int main(void)
{	
	static uint8_t CompletePro = 0;	//���һ�β���
	uint8_t UartRet;	//���ڷ��͵ķ�����Ϣ
	uint8_t InfoRet;	//��ȡ��Ϣ�ķ�����Ϣ
	
	SYS_Init();
	UART_Init();
	I2C_Init();

	POWERNFCMODE_OFF;
	//ָʾ��
	LED_LAMP_PIN = LED_ON;
	Delay_ms(20);
	LED_LAMP_PIN = LED_OFF;
	
	WDT_Init();
	SoftwareResetI2C();
	
	while(1)
	{
MainRepeat:
		if(TRAVELSWITCH == 0){
			Delay_ms(40);	//����
			if(TRAVELSWITCH == 0){
				POWERNFCMODE_ON;
				Delay_ms(50);
			}else
			{
				g_FeedDogFlag = 1;
				goto MainRepeat;
			}

			g_FeedDogFlag = 1;
		}
		else{
			POWERNFCMODE_OFF;
			CompletePro = 0;
			InitVariable();
			UART_Write(UART0, Complete_Buff, 6);
			Delay_ms(30);
			g_FeedDogFlag = 1;
			goto MainRepeat;
		}
		g_FeedDogFlag = 1;
		//������ɣ��ȴ��رտ��غ���һ�β���
		if(CompletePro == 1){
			goto MainRepeat;
		}
////////////////////////////////////////////////////////////////////////////////

		//ͨ��I2C��ȡ�����ж�ģ���Ƿ���������
		InfoRet = ReadNFCVersion();
		if(OK != InfoRet){
			POWERNFCMODE_OFF;
			UART_Write(UART0, NFCModeDetectFaild_Buff, 7);
			Delay_ms(10);
			CompletePro = 1;
			goto MainRepeat;
		}
		UART_Write(UART0, NFCModeDetectOK_Buff, 7);

////////////////////////////////////////////////////////////////////////////////
		 //�������Գ���
    		InfoRet = ReadCurrentInfo();
		if(InfoRet == 0xFF){
			POWERNFCMODE_OFF;
			Delay_ms(10);
			goto MainRepeat;
		}
		Delay_ms(2);
		//���͵�����Ϣ����λ��
		UartRet = SendCurrentDataToPC();	//���͵�����Ϣ
		if(OK != UartRet){
			goto MainRepeat;
		}

		if(InfoRet != 0x03){
			CompletePro = 1;
			goto MainRepeat;
		}


///////////////////////////////////////////////////////////////////////////////
		//��ȡ�ĸ���ǩ��Ϣ
		InfoRet = ReadNFCTagInfo();
		Delay_ms(2);
		if(OK != InfoRet){
			POWERNFCMODE_OFF;
			Delay_ms(10);
			goto MainRepeat;
		}
		//���ͱ�ǩ���ݵ���λ��
		UartRet = SendTagDataToPC();
		if(OK != UartRet){
			POWERNFCMODE_OFF;
			Delay_ms(10);
			goto MainRepeat;
		}
////////////////////////////////////////////////////////////////////////////////
			//��ȡ�汾��
			InfoRet = ReadNFCVersion();
			if(OK != InfoRet){
				POWERNFCMODE_OFF;
				Delay_ms(10);
				goto MainRepeat;
			}
			Delay_ms(2);

			//Delay_ms(100);

			//���Ͱ汾��Ϣ
			UartRet = SendVersionDataToPC();
			if(OK != UartRet){
				POWERNFCMODE_OFF;
				Delay_ms(10);
				goto MainRepeat;
			}


///////////////////////////////////////////////////////////////////////////////////
				#ifdef RESET_TEST
				//��ȡģ��������־
				InfoRet = ReadNFCResetFlag();
				if(OK != InfoRet){
					POWERNFCMODE_OFF;
					Delay_ms(10);
					goto MainRepeat;
				}
				Delay_ms(2);
				//����������־λ
				UartRet = SendNFCResetDataToPC();
				if(OK != UartRet){
					POWERNFCMODE_OFF;
					Delay_ms(10);
					goto MainRepeat;
				}
				#endif

/////////////////////////////////////////////////////////////////////////
/* ������� */
		CompletePro = 1;
	}
}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
