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
	static uint8_t CompletePro = 0;	//完成一次测试
	uint8_t UartRet;	//串口发送的返回信息
	uint8_t InfoRet;	//读取信息的返回信息
	
	SYS_Init();
	UART_Init();
	I2C_Init();

	POWERNFCMODE_OFF;
	//指示灯
	LED_LAMP_PIN = LED_ON;
	Delay_ms(20);
	LED_LAMP_PIN = LED_OFF;
	
	WDT_Init();
	SoftwareResetI2C();
	
	while(1)
	{
MainRepeat:
		if(TRAVELSWITCH == 0){
			Delay_ms(40);	//消抖
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
		//发送完成，等待关闭开关后下一次操作
		if(CompletePro == 1){
			goto MainRepeat;
		}
////////////////////////////////////////////////////////////////////////////////

		//通过I2C读取数据判断模块是否正常工作
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
		 //电流测试程序
    		InfoRet = ReadCurrentInfo();
		if(InfoRet == 0xFF){
			POWERNFCMODE_OFF;
			Delay_ms(10);
			goto MainRepeat;
		}
		Delay_ms(2);
		//发送电流信息到上位机
		UartRet = SendCurrentDataToPC();	//发送电流信息
		if(OK != UartRet){
			goto MainRepeat;
		}

		if(InfoRet != 0x03){
			CompletePro = 1;
			goto MainRepeat;
		}


///////////////////////////////////////////////////////////////////////////////
		//读取四个标签信息
		InfoRet = ReadNFCTagInfo();
		Delay_ms(2);
		if(OK != InfoRet){
			POWERNFCMODE_OFF;
			Delay_ms(10);
			goto MainRepeat;
		}
		//发送标签数据到上位机
		UartRet = SendTagDataToPC();
		if(OK != UartRet){
			POWERNFCMODE_OFF;
			Delay_ms(10);
			goto MainRepeat;
		}
////////////////////////////////////////////////////////////////////////////////
			//读取版本号
			InfoRet = ReadNFCVersion();
			if(OK != InfoRet){
				POWERNFCMODE_OFF;
				Delay_ms(10);
				goto MainRepeat;
			}
			Delay_ms(2);

			//Delay_ms(100);

			//发送版本信息
			UartRet = SendVersionDataToPC();
			if(OK != UartRet){
				POWERNFCMODE_OFF;
				Delay_ms(10);
				goto MainRepeat;
			}


///////////////////////////////////////////////////////////////////////////////////
				#ifdef RESET_TEST
				//读取模块重启标志
				InfoRet = ReadNFCResetFlag();
				if(OK != InfoRet){
					POWERNFCMODE_OFF;
					Delay_ms(10);
					goto MainRepeat;
				}
				Delay_ms(2);
				//发送重启标志位
				UartRet = SendNFCResetDataToPC();
				if(OK != UartRet){
					POWERNFCMODE_OFF;
					Delay_ms(10);
					goto MainRepeat;
				}
				#endif

/////////////////////////////////////////////////////////////////////////
/* 测试完成 */
		CompletePro = 1;
	}
}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
