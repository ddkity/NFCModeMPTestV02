#ifndef __SYSTEMCONFIG_H__
#define __SYSTEMCONFIG_H__

#define POWERNFCMODE_ON (P54 = 0)	/* ��NFCģ���ϵ� */
#define POWERNFCMODE_OFF (P54 = 1)	/* ��NFCģ��ϵ� */
#define TRAVELSWITCH (P52)			/* �г̿��ر�־ */

#define LED_LAMP_PIN (P36)
#define LED_ON (1)
#define LED_OFF (0)



void SYS_Init(void);
void UART_Init(void);
void I2C_Init(void);
void Delay_ms(uint32_t ms);
void Delay_100us(unsigned int delay_time);
void InitVariable(void);
void WDT_Init(void);
void SoftwareResetI2C(void);


#endif

