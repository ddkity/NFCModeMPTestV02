#include "main.h"

uint8_t g_UartReadBuffer[UARTBUFFERLEND] = {0};	//���ڽ��յ������ݻ���
volatile uint8_t g_UartRxOkFlag = 0;		//���ڽ������һ֡���ݵı�־��ʹ���������֮������
uint8_t g_CurrentInfo[CURRENTBUFF_LEN] = {0};

uint8_t UartSendToPCBuff[TOTAL_TAG_NUM][UARTBUFFERLEND] = {0};	//�����Ҫ���͵���λ���ı�ǩ��Ϣ
uint8_t SendVersionToPCBuff[VERSIONBUFF_LEN] = {0};	//�汾��Ϣ
uint8_t SendWDTResetFlagToPCBuff[WDTRESETBUFF_LEN] = {0};	//������Ϣ
uint8_t SendCurrentBuff[CURRENTBUFF_LEN] = {0};	//������Ϣ


static uint8_t UartReadTemp = 0;            /* UART����������ʱ��� */
uint8_t UartposStatus = UartNOP;          /* UART״̬��־λ */
uint8_t UartRecvLen = 0;	/* ���ڽ��յ��ܵ����ݳ��ȣ�ÿ����һ�ֽ�����1 */
uint8_t UartDataLen = 0;		/* ���ݰ������ݵĳ��� */

//�������ݵ�PC
void SendDataToPC(uint8_t cmd,uint8_t *data,uint8_t len)
{
	uint8_t i=0, j, crc = 0;
	uint8_t buff[UARTBUFFERLEND];
	buff[i++] = 0xFF;	//֡ͷ
	buff[i++] = 0xFF;	//֡ͷ
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
	buff[i++] = 0x3E;	//֡β
	UART_Write(UART0, buff, i);
}

void PackageUartData(uint8_t *Buffer, uint8_t id)
{
	uint8_t i;

	*Buffer++ = id;	//��ǩ���
	for(i = 0; i < 9; i++)
	{
		*Buffer++ = g_TagInfo[id][ 11 + i ];	//��������Ϣ��ʼ��ȡ��UID��Ϣ��Ҫ����λ����
	}
}

/* ר�Ž�����Ӧ��ǩ����Ϣ */
uint8_t AnalyzeTagUartReadData(uint8_t id)
{
	if((g_UartReadBuffer[2] != 0x80) || (g_UartReadBuffer[5] != 0x00))
	{
		return ERROR;
	}else
	{
		if((id < 5) && (g_UartReadBuffer[4] != id))	//��ǩ��Ŵ�0��ʼ
			return ERROR;
	}
	return OK;
}

/* �������������Ӧ����Ϣ */
uint8_t AnalyzeCMDUartReadData(uint8_t CMDNum)
{
	if((g_UartReadBuffer[2] != 0x80) || (g_UartReadBuffer[5] != 0x00))
	{
		return ERROR;
	}else
	{
		if(g_UartReadBuffer[4] != CMDNum)	//�����Ų����
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
		SendDataToPC(NFC_START_CMD, NULL, 0);	//���Ϳ�ʼ��������
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeTagUartReadData(0xFF);	//�������ڷ��ص�����
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
		ret = AnalyzeTagUartReadData(id);	//�������ڷ��ص�����
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

/* ���ͱ�ǩ��Ϣ��PC */
uint8_t SendTagDataToPC(void)
{
	uint8_t id;
	uint8_t ret;

	for(id = 0; id < 4; id++)
	{
		ret = SendTagInfoDataToPC(id);	//���ͱ�ǩ��Ϣ
		if(ERROR == ret){
			return ERROR;
		}
	}
	return OK;
}


/* ���Ͱ汾��Ϣ��PC */
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
		ret = AnalyzeCMDUartReadData(NFC_GETVERSION_CMD);	//�������ڷ��ص�����
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

/* ���͸�λ��Ϣ��PC */
uint8_t SendNFCResetDataToPC(void)
{
	volatile uint8_t cyclecnt;
	uint8_t ret;
	
	SendWDTResetFlagToPCBuff[0] = g_VersionInfo[3];
	cyclecnt = 4;

VerRepeat:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_GETRESETFLAG_CMD, SendWDTResetFlagToPCBuff, 1);	//���Ϳ�ʼ��������
		//while(0 == g_UartRxOkFlag);
		//ret = AnalyzeCMDUartReadData(NFC_GETRESETFLAG_CMD);	//�������ڷ��ص�����
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


/* ���͵�����Ϣ��PC */
uint8_t SendCurrentDataToPC(void)
{
	uint8_t ret;
	volatile uint8_t cyclecnt;
	
	memcpy(SendCurrentBuff, g_CurrentInfo, CURRENTBUFF_LEN);
	cyclecnt = 4;

TagRepeat1:
	while(cyclecnt != 0)
	{
		SendDataToPC(NFC_START_CMD, NULL, 0);	//���Ϳ�ʼ��������
		while(0 == g_UartRxOkFlag);
		ret = AnalyzeTagUartReadData(0xFF);	//�������ڷ��ص�����
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
		ret = AnalyzeCMDUartReadData(NFC_SENDCURRENT_CMD);	//�������ڷ��ص�����
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

/* �����жϺ��� */
/********************************************************************************************** 
	���ڽ������ݸ�ʽ
	֡ͷ(0xFF 0xFF) + ����(1Byte) + ��Ч���ݳ�(1Byte) +����Ч����(nByte)������У���(1Byte)
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
					if(UartReadTemp==0xFF)	//֡ͷ
					{
						g_UartReadBuffer[UartRecvLen++] = UartReadTemp; 
						UartposStatus = UartSOP2;
					}
					break;
				}
				case UartSOP2:
				{
					if(UartReadTemp==0xFF)	//֡ͷ
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
					g_UartRxOkFlag = 1;	//�������������ݵĵط�����������Ҫ��0
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




