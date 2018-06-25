#include "main.h"

uint8_t IN226DeviceCallBlockAddr = 0x00;
uint8_t IN226NowSendDataLen;		//实际发送的数据的长度
uint8_t IN226NowRecvDataLen;		//实际接收的数据长度
uint8_t IN226SendDataLen;		//需要发送的数据长度
uint8_t IN226RecvDataLen;		//需要接收的数据长度
volatile uint8_t IN226EndFlag = 0;	//I2C处理完成标志

uint8_t IN226WriteDataBuff[5] = {0};
uint8_t IN226ReadDataBuff[5] = {0};
uint8_t g_Have226I2CMode = 0;	//有I2C模块标志，在I2C中断中置1，用后清0
I2C_FUNC volatile s_I2C226HandlerFn = NULL;

void I2C0_IRQHandler(void)
{
    uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C0);
	g_Have226I2CMode = 1;

    if (I2C_GET_TIMEOUT_FLAG(I2C0)){
        /* Clear I2C Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    }else {
        if (s_I2C226HandlerFn != NULL)
            s_I2C226HandlerFn(u32Status);
    }
}

void I2C_INA226Rx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted and prepare SLA+W */
        I2C_SET_DATA(I2C0, IN226DeviceCallBlockAddr << 1);     /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
		if(IN226SendDataLen == 0)
		{
			I2C_SET_CONTROL_REG(I2C0, I2C_STA | I2C_SI);
		}
		else{
        	I2C_SET_DATA(I2C0, IN226WriteDataBuff[IN226NowSendDataLen++]);
        	I2C_SET_CONTROL_REG(I2C0, I2C_SI);
		}
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
		IN226EndFlag = 1;
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        if (IN226NowSendDataLen != IN226SendDataLen) {
            I2C_SET_DATA(I2C0, IN226WriteDataBuff[IN226NowSendDataLen++]);
            I2C_SET_CONTROL_REG(I2C0, I2C_SI);
        } else {
            I2C_SET_CONTROL_REG(I2C0, I2C_STA | I2C_SI);
        }
    } else if (u32Status == 0x10) {             /* Repeat START has been transmitted and prepare SLA+R */
        I2C_SET_DATA(I2C0, ((IN226DeviceCallBlockAddr << 1) | 0x01));   /* Write SLA+R to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x40) {             /* SLA+R has been transmitted and ACK has been received */
		I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
    }
	
	else if(u32Status == 0x50){
		if(IN226NowRecvDataLen != IN226RecvDataLen)
		{
			IN226ReadDataBuff[IN226NowRecvDataLen++] = I2C_GET_DATA(I2C0);
			I2C_SET_CONTROL_REG(I2C0, I2C_SI | I2C_AA);
		}
		else
		{
			I2C_SET_CONTROL_REG(I2C0, I2C_SI);
		}

	}
	else if (u32Status == 0x58) {             /* DATA has been received and NACK has been returned */
		IN226ReadDataBuff[IN226NowRecvDataLen++] = I2C_GET_DATA(I2C0);
		I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
		IN226EndFlag = 1;
    } 
	else if(u32Status == 0x30){
		IN226EndFlag = 1;
		I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
	}else {
		IN226EndFlag = 1;
		I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
    }
}

void I2C_INA226Tx(uint32_t u32Status)
{
    if (u32Status == 0x08) {                    /* START has been transmitted */
        I2C_SET_DATA(I2C0, IN226DeviceCallBlockAddr << 1);     /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    } else if (u32Status == 0x18) {             /* SLA+W has been transmitted and ACK has been received */
		if(IN226SendDataLen == 0)
		{
			I2C_SET_CONTROL_REG(I2C0, I2C_STA | I2C_SI);
		}
		else{
        	I2C_SET_DATA(I2C0, IN226WriteDataBuff[IN226NowSendDataLen++]);
        	I2C_SET_CONTROL_REG(I2C0, I2C_SI);
		}
    } else if (u32Status == 0x20) {             /* SLA+W has been transmitted and NACK has been received */
        I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
		IN226EndFlag = 1;
    } else if (u32Status == 0x28) {             /* DATA has been transmitted and ACK has been received */
        if (IN226NowSendDataLen != IN226SendDataLen) {
            I2C_SET_DATA(I2C0, IN226WriteDataBuff[IN226NowSendDataLen++]);
            I2C_SET_CONTROL_REG(I2C0, I2C_SI);
        } else {
        	IN226EndFlag = 1;
            I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
        }
    }else if(u32Status == 0x30){
			I2C_SET_CONTROL_REG(I2C0, I2C_STO | I2C_SI);
			IN226EndFlag = 1;
	} 
	else {
		IN226EndFlag = 1;
    }
}

void INA226_SetRegPointer(uint8_t addr,uint8_t reg)
{
	IN226NowSendDataLen = 0;
	IN226NowRecvDataLen = 0;
	IN226DeviceCallBlockAddr = addr;
	IN226WriteDataBuff[0] = reg;
	IN226SendDataLen = 1;
	s_I2C226HandlerFn = (I2C_FUNC)I2C_INA226Tx;
	I2C_START(I2C0);
	//Delay_ms(1);
	Delay_100us(30);
	if(g_Have226I2CMode == 0){
		return;
	}
	g_Have226I2CMode = 0;
	while(IN226EndFlag == 0);
	IN226EndFlag = 0;
	IN226SendDataLen = 0;
}

void INA226_SendData(uint8_t addr,uint8_t reg,uint16_t data)
{
	uint8_t temp=0;
	IN226NowSendDataLen = 0;
	IN226NowRecvDataLen = 0;
	IN226DeviceCallBlockAddr = addr;
	IN226WriteDataBuff[0] = reg;
	temp = (uint8_t)(data>>8);
	IN226WriteDataBuff[1] = temp;
	temp = (uint8_t)(data&0x00FF);
	IN226WriteDataBuff[2] = temp;
	IN226SendDataLen = 3;
	s_I2C226HandlerFn = (I2C_FUNC)I2C_INA226Tx;
	I2C_START(I2C0);
	Delay_100us(30);
	if(g_Have226I2CMode == 0){
		return;
	}
	g_Have226I2CMode = 0;
	while(IN226EndFlag == 0);
	IN226EndFlag = 0;
	IN226SendDataLen = 0;
}

uint16_t INA226_ReadData(uint8_t addr)
{
	uint16_t temp=0;
	IN226NowSendDataLen = 0;
	IN226NowRecvDataLen = 0;
	IN226DeviceCallBlockAddr = addr;
	IN226SendDataLen = 0;
	IN226RecvDataLen = 2;	//或者是等于1也可以?因为后面还没有一个读的动作
	s_I2C226HandlerFn = (I2C_FUNC)I2C_INA226Rx;
	I2C_START(I2C0);
	//Delay_ms(1);
	Delay_100us(30);
	if(g_Have226I2CMode == 0){
		return 0;
	}
	g_Have226I2CMode = 0;
	while(IN226EndFlag == 0);
	IN226EndFlag = 0;
	IN226RecvDataLen = 0;
	temp = IN226ReadDataBuff[0];
	temp<<=8;	
	temp |= IN226ReadDataBuff[1];
	return temp;
}


void INA226_Init(void)
{	
	INA226_SendData(INA226DeviceAddr, CFG_REG, 0x43FF);
	INA226_SendData(INA226DeviceAddr, CAL_REG, 1024);
}

uint16_t INA226_GetCurrent(void)
{
	volatile uint16_t temp=0;	
    	uint8_t addr = INA226DeviceAddr;
	INA226_SetRegPointer(addr,SV_REG);
	temp = INA226_ReadData(addr);
	if(temp&0x8000)
	{
		temp = ~(temp - 1);	
		temp = (temp*25 + 5) / 10 / 5;		
	}
	else
	{
		INA226_SetRegPointer(addr,CUR_REG);
		temp = INA226_ReadData(addr);
		temp = (temp&0x7fff)/2;
	}
	return temp;
}

uint16_t INA226_GetVoltage(void)
{
	volatile uint16_t temp=0;
    	uint8_t addr = INA226DeviceAddr;
	INA226_SetRegPointer(addr,BV_REG);
	temp = INA226_ReadData(addr);
	temp = (temp*125 + 50)/100;
	return temp;	
}

/* 读取休眠时的电流 */
uint32_t ReadStaCurrent(void)
{	
	uint16_t Stacur[6];
	uint32_t Staaverage;

	INA226_Init();
	Delay_ms(7);
  Stacur[0] = INA226_GetCurrent();
	Stacur[1] = INA226_GetCurrent();
	Stacur[2] = INA226_GetCurrent();
	Stacur[3] = INA226_GetCurrent();
	Stacur[4] = INA226_GetCurrent();
	Stacur[5] = INA226_GetCurrent();

	Staaverage = Stacur[1] + Stacur[2] + Stacur[3] + Stacur[4] + Stacur[5];
	Staaverage = Staaverage/5;

	return Staaverage;
}

/* 读取工作时的电流 */
uint32_t ReadWorkCurrent(void)
{
	uint16_t Workcur[6];
	uint32_t Woreaverage;

	ReadDataFormNFCMode(I2CReadWorkCurrentCMD);

	//判断是否发送命令成功
	if(g_I2CRecvDataBuffer[3] != I2CReadWorkCurrentCMD + 1){
		memset(g_I2CRecvDataBuffer, 0, I2CRECVDATAFROMNFCLEN);
		return ERROR;
	}
	memset(g_I2CRecvDataBuffer, 0, I2CRECVDATAFROMNFCLEN);
	
	INA226_Init();
	Delay_ms(13);
   	 Workcur[0] = INA226_GetCurrent();
	Workcur[1] = INA226_GetCurrent();
	Workcur[2] = INA226_GetCurrent();
	Workcur[3] = INA226_GetCurrent();
	Workcur[4] = INA226_GetCurrent();
	Workcur[5] = INA226_GetCurrent();

	Woreaverage = Workcur[1] + Workcur[2] + Workcur[3] + Workcur[4] + Workcur[5];
	Woreaverage = Woreaverage/5;
	return Woreaverage;
}


uint8_t ReadCurrentInfo(void)
{
	uint32_t StaCurrent;
	uint32_t WorkCurrent;
	uint8_t i;

	StaCurrent = ReadStaCurrent();

	for(i = 0; i < 2; i++)
	{
		if((StaCurrent <= STAMINCURRENT) || (StaCurrent >= STAMAXCURRENT)){
			StaCurrent = ReadStaCurrent();
		}
		else{
			break;
		}
			
	}
	
	WorkCurrent = ReadWorkCurrent();
	for(i = 0; i < 2; i++)
	{
		if((WorkCurrent <= WORKMINCURRENT) || (WorkCurrent >= WORKMAXCURRENT)){
			WorkCurrent = ReadWorkCurrent();
		}
		else{
			break;
		}
	}

	//如果电流测试都为1，可能I2C挂起了，强行重启
	if((1 == StaCurrent) || (1 == WorkCurrent) || (0 == StaCurrent) || (0 == WorkCurrent)){
		SoftwareResetI2C();
		return 0xFF;
	}

	if((StaCurrent > STAMINCURRENT) && (StaCurrent < STAMAXCURRENT)){
		//g_CurrentInfo[0]的第二位表示静态电流的状态,1表示成功，0表示失败
		g_CurrentInfo[0] = g_CurrentInfo[0] | (1 << 1);
	}
	else{
		g_CurrentInfo[0] = g_CurrentInfo[0] | (0 << 1);
	}

	if((WorkCurrent > WORKMINCURRENT) && (WorkCurrent < WORKMAXCURRENT)){
		//g_CurrentInfo[0]的第一位表示工作电流的状态,1表示成功，0表示失败
		g_CurrentInfo[0] = g_CurrentInfo[0] | (1 << 0);
	}else{
		g_CurrentInfo[0] = g_CurrentInfo[0] | (0 << 0);
	}
	
	g_CurrentInfo[1] = ((StaCurrent >> 8) & 0xFF);		//静态电流高八位;
	g_CurrentInfo[2] = (StaCurrent & 0xFF); 			//静态电流低八位;
	g_CurrentInfo[3] = ((WorkCurrent >> 8) & 0xFF);		//动态电流高八位;
	g_CurrentInfo[4] = (WorkCurrent & 0xFF);			//动态电流低八位;

	return g_CurrentInfo[0];	//返回检测状态,0x03是测试通过的，0x02位工作电流失败,0x01为静态电流失败，0x00为静态动态电流都测试失败
}


