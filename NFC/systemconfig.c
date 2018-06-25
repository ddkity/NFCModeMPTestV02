#include "main.h"

void Delay_ms(uint32_t ms)		//5ms
{
	int i, j;
	for(i = 0; i < ms; i++)
	{
		for(j = 0; j < 12000; j++)
		{
			;
		}
	}
}

void Delay_100us(unsigned int delay_time)		//0.1ms*delay_time
{
    unsigned int i;
    for(i = 0; i < delay_time; i++)
    {
        CLK_SysTickDelay(100);
    }
}

void SYS_Init(void)
{
	/* Unlock protected registers */
	SYS_UnlockReg();

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init System Clock                                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Set P5 multi-function pins for XTAL1 and XTAL2 */
	SYS->P5_MFP &= ~(SYS_MFP_P50_Msk | SYS_MFP_P51_Msk);
	SYS->P5_MFP |= (SYS_MFP_P50_XT1_IN | SYS_MFP_P51_XT1_OUT);

	/* Enable external 12MHz XTAL, HIRC */
	CLK->PWRCTL |= CLK_PWRCTL_XTL12M | CLK_PWRCTL_HIRCEN_Msk | CLK_PWRCTL_LIRCEN_Msk;

	/* Waiting for clock ready */
	CLK_WaitClockReady(CLK_STATUS_XTLSTB_Msk | CLK_STATUS_HIRCSTB_Msk | CLK_STATUS_LIRCSTB_Msk);

	/* Switch HCLK clock source to XTL */
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_XTAL,CLK_CLKDIV_HCLK(1));

	/* STCLK to XTL STCLK to XTL */
	CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_XTAL);

	/* Enable IP clock */
	CLK_EnableModuleClock(UART0_MODULE);
	CLK_EnableModuleClock(I2C0_MODULE);
	CLK_EnableModuleClock(I2C1_MODULE);
	CLK_EnableModuleClock(WDT_MODULE);

	/* Select IP clock source */
	CLK_SetModuleClock(UART0_MODULE,CLK_CLKSEL1_UARTSEL_XTAL,CLK_CLKDIV_UART(1));
	CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init I/O Multi-function                                                                                 */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Set P3.4 and P3.5 for I2C SDA and SCL */
	SYS->P3_MFP &= ~(SYS_MFP_P34_Msk | SYS_MFP_P35_Msk);
	SYS->P3_MFP = SYS_MFP_P34_I2C0_SDA | SYS_MFP_P35_I2C0_SCL;

	SYS->P2_MFP &= ~(SYS_MFP_P23_Msk | SYS_MFP_P22_Msk);
	SYS->P2_MFP = SYS_MFP_P23_I2C1_SDA | SYS_MFP_P22_I2C1_SCL;

	/* Set P1 multi-function pins for UART RXD and TXD */
	SYS->P0_MFP &= ~(SYS_MFP_P00_Msk | SYS_MFP_P01_Msk);
	SYS->P0_MFP |= (SYS_MFP_P01_UART0_RXD | SYS_MFP_P00_UART0_TXD);

	/* Lock protected registers */
	SYS_LockReg();

	/* Update System Core Clock */
	SystemCoreClockUpdate();

	GPIO_SetMode(P5, BIT2, GPIO_MODE_INPUT);	/* 行程开关标志 */
	GPIO_SetMode(P5, BIT4, GPIO_MODE_OUTPUT);	/* NFC模块供电开关 */
	GPIO_SetMode(P3, BIT6, GPIO_MODE_OUTPUT);	/* 指示灯 */
}

void UART_Init(void)
{
	CLK->APBCLK |= CLK_APBCLK_UART0CKEN_Msk;
	CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
	CLK->CLKSEL1 |= (0x0 << CLK_CLKSEL1_UARTSEL_Pos);
	SYS->P1_MFP &= ~(SYS_MFP_P00_Msk | SYS_MFP_P01_Msk);
	SYS->P1_MFP |= (SYS_MFP_P01_UART0_RXD | SYS_MFP_P00_UART0_TXD);
	UART_Open(UART0,115200);
	UART_ENABLE_INT(UART0, (UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk));
	NVIC_EnableIRQ(UART0_IRQn);	
}

void I2C_Init(void)
{
	/* Open I2C module and set bus clock */
	I2C_Open(I2C0, 100000);
	I2C_EnableInt(I2C0);
	NVIC_EnableIRQ(I2C0_IRQn);

	/* Open I2C module and set bus clock */
	I2C_Open(I2C1, 100000);
	I2C_EnableInt(I2C1);
	NVIC_EnableIRQ(I2C1_IRQn);
}


void WDT_IRQHandler(void)
{
	if(g_FeedDogFlag == 1)
	{
    	WWDT_RELOAD_COUNTER();
    	WWDT_CLEAR_INT_FLAG();
	}

	WWDT_CLEAR_INT_FLAG();
	g_FeedDogFlag = 0;
}

void WDT_Init(void)
{
	SYS_UnlockReg();
	WWDT_Open(WWDT_PRESCALER_192, 0x03, TRUE);	//打开看门狗，让程序重启
	NVIC_EnableIRQ(WDT_IRQn);
}

/* I2C0 */
void SoftwareResetI2C(void)
{
	uint8_t i;
	//SYS_UnlockReg();
	//SYS->IPRST0 |= 0x00000001UL;	//bit0 置1重启芯片
	//重启I2C
	NVIC_DisableIRQ(I2C0_IRQn);
	I2C_DisableInt(I2C0);
	I2C_Close(I2C0);

	NVIC_DisableIRQ(I2C1_IRQn);
	I2C_DisableInt(I2C1);
	I2C_Close(I2C1);
/*   I2C0   */
	
	SYS->P3_MFP &= ~(SYS_MFP_P34_Msk | SYS_MFP_P35_Msk);
	SYS->P3_MFP = SYS_MFP_P34_GPIO | SYS_MFP_P35_GPIO;
	GPIO_SetMode(P3, BIT4 | BIT5, GPIO_MODE_OUTPUT);
	for(i = 0; i < 9; i++)
	{
		P34 = 1;
		P35 = 0;
		CLK_SysTickDelay(6);
		P35 = 1;
		CLK_SysTickDelay(6);
	}
	//LED_LAMP_PIN = LED_ON;
	//Delay_ms(2);
	//LED_LAMP_PIN = LED_OFF;

	CLK_EnableModuleClock(I2C0_MODULE);
	SYS->P3_MFP &= ~(SYS_MFP_P34_Msk | SYS_MFP_P35_Msk);
	SYS->P3_MFP = SYS_MFP_P34_I2C0_SDA | SYS_MFP_P35_I2C0_SCL;


	/*	 I2C1	*/
	SYS->P2_MFP &= ~(SYS_MFP_P22_Msk | SYS_MFP_P23_Msk);
	SYS->P2_MFP = SYS_MFP_P22_GPIO | SYS_MFP_P23_GPIO;
	GPIO_SetMode(P2, BIT2 | BIT3, GPIO_MODE_OUTPUT);
	for(i = 0; i < 9; i++)
	{
		P23 = 1;
		P22 = 0;	//clk
		CLK_SysTickDelay(6);
		P22 = 1;
		CLK_SysTickDelay(6);
	}
	//LED_LAMP_PIN = LED_ON;
	//Delay_ms(2);
	//LED_LAMP_PIN = LED_OFF;

	CLK_EnableModuleClock(I2C1_MODULE);
	SYS->P2_MFP &= ~(SYS_MFP_P23_Msk | SYS_MFP_P22_Msk);
	SYS->P2_MFP = SYS_MFP_P23_I2C1_SDA | SYS_MFP_P22_I2C1_SCL;
	I2C_Init();
}

//初始化所有的全局变量
void InitVariable(void)
{
	int i;

	//NFC模块相关的全局变量
	for(i = 0; i < 4; i++)
	{
		memset(g_TagInfo[i], 0, I2CRECVDATAFROMNFCLEN);
	}
	memset(g_VersionInfo, 0, I2CRECVDATAFROMNFCLEN);
	memset(g_NFCResetInfo, 0, I2CRECVDATAFROMNFCLEN);
	
	g_HaveI2CMode = 0;
	g_Have226I2CMode = 0;
	
	memset(g_CurrentInfo, 0, CURRENTBUFF_LEN);
	
	memset(g_I2CRecvDataBuffer, 0, I2CRECVDATAFROMNFCLEN);
	memset(IN226WriteDataBuff, 0, 5);
	memset(IN226ReadDataBuff, 0, 5);

	//串口相关的全局变量
	g_UartRxOkFlag = 0;
	memset(g_UartReadBuffer, 0, UARTBUFFERLEND);
}


