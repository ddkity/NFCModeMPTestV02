#include "main.h"

uint8_t g_UartReadBuffer[UARTBUFFERLEND] = {0};	//串口接收到的数据缓存
volatile uint8_t g_UartRxOkFlag = 0;		//串口接收完成一帧数据的标志，使用完成数据之后清零
uint8_t g_CurrentInfo[CURRENTBUFF_LEN] = {0};

uint8_t UartSendToPCBuff[TOTAL_TAG_NUM][UARTBUFFERLEND] = {0};	//存放需要发送到上位机的标签信息
uint8_t SendVersionToPCBuff[VERSIONBUFF_LEN] = {0};	//版本信息
uint8_t SendWDTResetFlagToPCBuff[WDTRESETBUFF_LEN] = {0};	//重启信息
uint8_t SendCurrentBuff[CURRENTBUFF_LEN] = {0};	//电流信息


static uint8_t UartReadTemp = 0;            /* UART接收数据暂时存放 */
uint8_t UartposStatus = UartNOP;          /* UART状态标志位 */
uint8_t UartRecvLen = 0;	/* 串口接收的总的数据长度，每接收一字节增加1 */
uint8_t UartDataLen = 0;		/* 数据包中数据的长度 */

//发送数据到PC
void SendDataToPC(uint8_t cmd,uint8_t *data,uint8_t len)
{
	uint8_t i=0, j, crc = 0;
	uint8_t buff[UARTBUFFERLEND];
	buff[i++] = 0xFF;	//帧头
	buff[i++] = 0xFF;	//帧头
	buff[i++] = cmd;
	buff[i++] = len;
	for(j=0;j<len;j++)
	{
		buff[i++] = data[j];
		crc ^= data[j];
	}
	crc ^= cmd;
	crc ^= len;
	buff[i++] = crc;
	buff[i++] = 0x3E;	//帧尾
	UART_Write(UART0, buff, i);
}

void PackageUartData(uint8_t *Buffer, uint8_t id)
{
	uint8_t i;

	*Buffer++ = id;	//标签序号
	for(i = 0; i < 9; i++)
	{
		*Buffer++ = g_TagInfo[id][ 11 + i ];	//从滤网信息开始读取，UID信息不要传上位机了
	}
}

/* 专门解析回应标签的信息 */
uint8_t AnalyzeTagUartReadData(uint8_t id)
{
	if((g_UartReadBuffer[2] != 0x80) || (g_UartReadBuffer[5] != 0x00))
	{
		return ERROR;
	}else
	{
		if((id < 5) && (g_UartReadBuffer[4] != id))	//标签序号从0开始
			return ERROR;
	}
	return OK;
}

/* 解析其他命令回应的信息 */
uint8_t AnalyzeCMDUartReadData(uint8_t CMDNum)
{
	if((g_UartReadBuffer[2] != 0x80) || (g_UartReadBuffer[5] != 0x00))
	{
		return ERROR;
	}else
	{
		if(g_UartReadBuffer[4] != CMDNum)	//命令标号不相等
			return ERROR;
	}
	return OK;
}

uint8_t SendTagInfoDataToPC(uint8_t id)
{
	uint8_t ret;
	volatile uint8_t cyclecnt;
	uint8_t TempBuff[10] = {0};
	
	PackageUartData(TempBuff, id);
	memcpy(UartSendToPCBuff[id], TempBuff, 10);
	
	cyclecnt = 4;

TagRepeat1:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_START_CMD, NULL, 0);	//发送开始产测命令
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeTagUartReadData(0xFF);	//解析串口返回的数据
		g_UartRxOkFlag = 0;
		if(ret == OK){
			cyclecnt = 0;
		}
		else{
			cyclecnt--;
			goto TagRepeat1;
		}
	}

	if(ret == ERROR){
		return ERROR;
	}
	
	cyclecnt = 4;
TagRepeat2:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_SEND_INFO_CMD, UartSendToPCBuff[id], 10);
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeTagUartReadData(id);	//解析串口返回的数据
		g_UartRxOkFlag = 0;
		if(ret == OK){
			cyclecnt = 0;
		}
		else{
			cyclecnt--;
			goto TagRepeat2;
		}
	}

	if(ret == ERROR){
		return ERROR;
	}

	return ret;
}

/* 发送标签信息到PC */
uint8_t SendTagDataToPC(void)
{
	uint8_t id;
	uint8_t ret;

	for(id = 0; id < 4; id++)
	{
		ret = SendTagInfoDataToPC(id);	//发送标签信息
		if(ERROR == ret){
			return ERROR;
		}
	}
	return OK;
}


/* 发送版本信息到PC */
uint8_t SendVersionDataToPC(void)
{
	volatile uint8_t cyclecnt;
	uint8_t ret = ERROR;
	
	SendVersionToPCBuff[0] = g_VersionInfo[3];
	SendVersionToPCBuff[1] = g_VersionInfo[4];
	cyclecnt = 4;

VerRepeat:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_GETVERSION_CMD, SendVersionToPCBuff, 2);
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeCMDUartReadData(NFC_GETVERSION_CMD);	//解析串口返回的数据
		Delay_ms(2);
		g_UartRxOkFlag = 0;
		if(ret == OK){
			cyclecnt = 0;
		}
		else{
			cyclecnt--;
			goto VerRepeat;
		}
	}
	
	if(ret == ERROR){
		return ERROR;
	}

	return ret;
	
}

/* 发送复位信息到PC */
uint8_t SendNFCResetDataToPC(void)
{
	volatile uint8_t cyclecnt;
	uint8_t ret;
	
	SendWDTResetFlagToPCBuff[0] = g_VersionInfo[3];
	cyclecnt = 4;

VerRepeat:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_GETRESETFLAG_CMD, SendWDTResetFlagToPCBuff, 1);	//发送开始产测命令
		//while(0 == g_UartRxOkFlag);
		//ret = AnalyzeCMDUartReadData(NFC_GETRESETFLAG_CMD);	//解析串口返回的数据
		Delay_ms(2);
		g_UartRxOkFlag = 0;
		if(ret == OK){
			cyclecnt = 0;
		}
		else{
			cyclecnt--;
			goto VerRepeat;
		}
	}

	if(ret == ERROR){
		return ERROR;
	}

	return ret;
}


/* 发送电流信息到PC */
uint8_t SendCurrentDataToPC(void)
{
	uint8_t ret;
	volatile uint8_t cyclecnt;
	
	memcpy(SendCurrentBuff, g_CurrentInfo, CURRENTBUFF_LEN);
	cyclecnt = 4;

TagRepeat1:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_START_CMD, NULL, 0);	//发送开始产测命令
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeTagUartReadData(0xFF);	//解析串口返回的数据
		g_UartRxOkFlag = 0;
		if(ret == OK){
			cyclecnt = 0;
		}
		else{
			cyclecnt--;
			goto TagRepeat1;
		}
	}

	if(ret == ERROR){
		return ERROR;
	}
	
	cyclecnt = 4;
TagRepeat2:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_SENDCURRENT_CMD, SendCurrentBuff, CURRENTBUFF_LEN);
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeCMDUartReadData(NFC_SENDCURRENT_CMD);	//解析串口返回的数据
		g_UartRxOkFlag = 0;
		if(ret == OK){
			cyclecnt = 0;
		}
		else{
			cyclecnt--;
			goto TagRepeat2;
		}
	}

	if(ret == ERROR){
		return ERROR;
	}

	return ret;
}

/* 串口中断函数 */
/********************************************************************************************** 
	串口接收数据格式
	帧头(0xFF 0xFF) + 命令(1Byte) + 有效数据长(1Byte) +　有效数据(nByte)　＋　校验和(1Byte)
**********************************************************************************************/
void UART0_IRQHandler(void)
{
    uint32_t u32IntSts= UART0->INTSTS;
	
    if(u32IntSts & UART_INTSTS_RDAINT_Msk) 
	{
        while(UART0->INTSTS & UART_INTSTS_RDAIF_Msk) 
        {
            UartReadTemp = UART0->DAT;
			switch(UartposStatus)
			{
				case UartNOP:
				{
					if(g_UartRxOkFlag)
						break;
					else
						UartposStatus = UartSOP1;
				}
				case UartSOP1:
				{
					if(UartReadTemp==0xFF)	//帧头
					{
						g_UartReadBuffer[UartRecvLen++] = UartReadTemp; 
						UartposStatus = UartSOP2;
					}
					break;
				}
				case UartSOP2:
				{
					if(UartReadTemp==0xFF)	//帧头
					{
						g_UartReadBuffer[UartRecvLen++] = UartReadTemp; 
						UartposStatus = UartCMD;
					}
					break;
				}
				case UartCMD:
				{
					g_UartReadBuffer[UartRecvLen++] = UartReadTemp;
					UartposStatus = UartLEN;
					break;
				}
				case UartLEN:
				{
					g_UartReadBuffer[UartRecvLen++] = UartReadTemp;
					UartDataLen = UartReadTemp;
					UartposStatus = UartDATA;
					break;
				}
				case UartDATA:
				{
					if(UartDataLen--)
					{
						g_UartReadBuffer[UartRecvLen++] = UartReadTemp;
						break;
					}
					else
						UartposStatus = UartXOR;
				}

				case UartXOR:
				{
					g_UartReadBuffer[UartRecvLen++] = UartReadTemp;
					g_UartRxOkFlag = 1;	//其他处理串口数据的地方处理完了需要清0
					UartRecvLen = 0;
					UartDataLen = 0;
					UartposStatus = UartNOP;
				}
				default:
					break;
			}
        }
    }
}




