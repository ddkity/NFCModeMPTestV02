#include "main.h"


//ȫ�ֱ����Ķ���
uint8_t g_TagInfo[TOTAL_TAG_NUM][I2CRECVDATAFROMNFCLEN] = {0};
uint8_t g_VersionInfo[I2CRECVDATAFROMNFCLEN] = {0};
uint8_t g_NFCResetInfo[I2CRECVDATAFROMNFCLEN] = {0};


uint8_t g_HaveI2CMode = 0;	//��I2Cģ���־����I2C�ж�����1���ú���0
uint8_t g_I2CRecvDataBuffer[I2CRECVDATAFROMNFCLEN] = {0};	/* ��NFCģ���ȡ������ */
//uint8_t Complete_Buff[6] = {0xFF, 0xFF, 0xaa, 0x00, 0x01, 0x3E};
//uint8_t NFCModeDetectOK_Buff[7] = {0xFF, 0xFF, 0x07, 0x01, 0x00, 0x06, 0x3E};
//uint8_t NFCModeDetectFaild_Buff[7] = {0xFF, 0xFF, 0x07, 0x01, 0x01, 0x07, 0x3E};

volatile uint8_t  g_FeedDogFlag = 1;
//uint8_t g_TagUIDFromPC[TOTAL_TAG_NUM][8] = {0};	//�������λ������������UID��Ϣ

/* ��NFC��α��оģ�鷢�͵����� */
uint8_t I2CWriteDataBuffer[8][6] =
                            {
				{0xAA, 0x04, 0x01, 0xE6, 0x01, 0xE2},	/* tag1 */
				{0xAA, 0x04, 0x02, 0xE6, 0x01, 0xE1},	/* tag2 */
				{0xAA, 0x04, 0x03, 0xE6, 0x01, 0xE0},	/* tag3 */
				{0xAA, 0x04, 0x04, 0xE6, 0x01, 0xE7},	/* tag4 */
				{0xAA, 0x04, 0x05, 0xE6, 0x01, 0xE6},	//�汾��Ϣ��ѯ����
				{0xAA, 0x04, 0x06, 0xE6, 0x01, 0xE5},	//ģ�鸴λ��Ϣ��ȡ
				{0xAA, 0x04, 0x07, 0xE6, 0x01, 0xE4},	//ʹNFCģ������������ģʽ
				{0xAA, 0x04, 0x08, 0xE0, 0x01, 0xED}	//�°汾��Ϣ��ѯ����
                            };										
uint8_t *pI2CWriteData;

uint8_t I2CSendDataLen = 0;	/* I2C���͵������ݳ��ȣ�ÿ����һ�ֽ�����1 */
uint8_t I2CRecvDataLen = 0; /* I2C���յ������ݳ��ȣ�ÿ����һ�ֽ�����1 */
volatile uint8_t I2CRWEndFlag = 0;	/* ��NFCģ���I2C������ɱ�־ ���������ȴ������־�����,�����volatile������������while(I2CRWEndFlag == 0)�лᱻ�Ż� */
I2C_FUNC volatile s_I2CNFCHandlerFn = NULL;

void I2C1_IRQHandler(void)
{
    uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C1);
	g_HaveI2CMode = 1;

    if (I2C_GET_TIMEOUT_FLAG(I2C1)){
        /* Clear I2C Timeout Flag */
        I2C_ClearTimeoutFlag(I2C1);
    }else {
        if (s_I2CNFCHandlerFn != NULL)
            s_I2CNFCHandlerFn(u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*	I2C Rx Callback Function                                                                               */
/*	��ȡNFCģ���I2C�ص����� */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterRx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted and prepare SLA+W */
        I2C_SET_DATA(I2C1, NFCModeDeviceAddr << 1);     /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
        I2C_SET_DATA(I2C1, pI2CWriteData[I2CSendDataLen++]);
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
		I2CRWEndFlag = 1;
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        if (I2CSendDataLen != I2CSENDDATATONFCLEN){
            I2C_SET_DATA(I2C1, pI2CWriteData[I2CSendDataLen++]);
            I2C_SET_CONTROL_REG(I2C1, I2C_SI);
        } else {
			Delay_100us(10);
            I2C_SET_CONTROL_REG(I2C1, I2C_STA | I2C_SI);
        }
    } else if (u32Status == 0x10) {             /* Repeat START has been transmitted and prepare SLA+R */
        I2C_SET_DATA(I2C1, ((NFCModeDeviceAddr << 1) | 0x01));   /* Write SLA+R to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x40) {             /* SLA+R has been transmitted and ACK has been received */
		I2C_SET_CONTROL_REG(I2C1, I2C_SI | I2C_AA);
    }
	else if(u32Status == 0x50){
		if(I2CRecvDataLen != I2CRECVDATAFROMNFCLEN)
		{
			g_I2CRecvDataBuffer[I2CRecvDataLen++] = I2C_GET_DATA(I2C1);
			I2C_SET_CONTROL_REG(I2C1, I2C_SI | I2C_AA);
		}
		else
		{
			I2C_SET_CONTROL_REG(I2C1, I2C_SI);
		}
	}
	else if (u32Status == 0x58) {             /* DATA has been received and NACK has been returned */
		g_I2CRecvDataBuffer[I2CRecvDataLen++] = I2C_GET_DATA(I2C1);
		I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
        I2CRWEndFlag = 1;
		I2CRecvDataLen = 0;
    }
	else if(u32Status == 0x30){
		I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
        I2CRWEndFlag = 1;
		I2CRecvDataLen = 0;
	}else {
		I2CRWEndFlag = 1;
		I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Tx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterTx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted */
        I2C_SET_DATA(I2C1, NFCModeDeviceAddr << 1);     /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
        I2C_SET_DATA(I2C1, pI2CWriteData[I2CSendDataLen++]); 
        I2C_SET_CONTROL_REG(I2C1, I2C_SI);
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
		I2CRWEndFlag = 1;
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        if (I2CSendDataLen != I2CSENDDATATONFCLEN) {
            I2C_SET_DATA(I2C1, pI2CWriteData[I2CSendDataLen++]);
            I2C_SET_CONTROL_REG(I2C1, I2C_SI);
        } else {
            I2C_SET_CONTROL_REG(I2C1, I2C_STO | I2C_SI);
            I2CRWEndFlag = 1;
        }
    } else {
    	I2CRWEndFlag = 1;
    }
}

uint8_t ReadDataFormNFCMode(uint8_t id)
{
	uint8_t cycle;
	uint8_t i;

	cycle = 2;	//ѭ����ȡ���Σ���ȡ���ɹ����˳�

Repeat:	
	while(cycle > 0)
	{
		pI2CWriteData = I2CWriteDataBuffer[id];
		I2CSendDataLen = 0;
		I2CRWEndFlag = 0;
		s_I2CNFCHandlerFn = (I2C_FUNC)I2C_MasterRx;

		 I2C_START(I2C1);

		Delay_ms(1);
		if(g_HaveI2CMode == 0)
		{
			cycle--;
			goto Repeat;
		}
		g_HaveI2CMode = 0;
		while(0 == I2CRWEndFlag);

		if(id < 4)	//�汾�Ų�����
		{
			//����decryption
			for(i = 0; i < 18; i++)
			{
				g_I2CRecvDataBuffer[i + 2] = g_I2CRecvDataBuffer[i + 2]^I2CWriteDataBuffer[id][3];
			}
		}

		#if 0
		//���ܺ�
		printf("After Decryption NFC Reader-->HostMCU:");
		for(i = 0; i < 21; i++)
		{
			printf("%x ", g_I2CRecvDataBuffer[i]);
		}
		
		printf("\n\r");
		printf("\n\r");
		#endif
		
		//�ж϶�ȡ�������Ƿ�����ȷ�����ݣ�������ǵĻ����¶��������Ĵ�
		if((g_I2CRecvDataBuffer[0] == 0) && (g_I2CRecvDataBuffer[1] == 0)){
			cycle--;
			g_FeedDogFlag = 1;
			if(cycle != 0){
				Delay_ms(60);
			}
		}else if((g_I2CRecvDataBuffer[0] == 0xF0) && (g_I2CRecvDataBuffer[1] == 0xF0)){
			cycle--;
			g_FeedDogFlag = 1;
			if(cycle != 0){
				Delay_ms(60);
			}
		}else{
			cycle = 0;
		}
	}

	//������Ƕ��������ݣ���ΪI2C�����Ѿ������ˣ�����оƬ
	if((g_I2CRecvDataBuffer[0] != 0xAA) && (g_I2CRecvDataBuffer[1] != 0x13)){
		SoftwareResetI2C();
		return ERROR;
	}
	return OK;
}

/* ��ȡ�ĸ���ǩ����Ϣ */
uint8_t ReadNFCTagInfo(void)
{
	uint8_t id;
	uint8_t ret;

	for(id = 0; id < 4; id++)
	{
		ret = ReadDataFormNFCMode(id);
		memcpy(g_TagInfo[id], &g_I2CRecvDataBuffer[0], I2CRECVDATAFROMNFCLEN);
		memset(&g_I2CRecvDataBuffer[0], 0, I2CRECVDATAFROMNFCLEN);
		if(OK != ret){
			return ERROR;
		}
	}
	return OK;
}

/* ��ȡ�汾�� */
uint8_t ReadNFCVersion(void)
{
	uint8_t ret;
	
	ret = ReadDataFormNFCMode(I2CReadVersionCMD);
	memcpy(g_VersionInfo, &g_I2CRecvDataBuffer[0], I2CRECVDATAFROMNFCLEN);
	memset(&g_I2CRecvDataBuffer[0], 0, I2CRECVDATAFROMNFCLEN);
	if(OK != ret){
		return ERROR;
	}

	return OK;
}

/* ��ȡ������־λ */
uint8_t ReadNFCResetFlag(void)
{
	uint8_t ret;
	
	ret = ReadDataFormNFCMode(I2CReadResetFlagCMD);
	memcpy(g_NFCResetInfo, &g_I2CRecvDataBuffer[0], I2CRECVDATAFROMNFCLEN);
	memset(&g_I2CRecvDataBuffer[0], 0, I2CRECVDATAFROMNFCLEN);

	if(OK != ret){
		return ERROR;
	}
	return OK;
}



