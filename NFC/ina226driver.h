#ifndef __INA226DRIVER_H__
#define __INA226DRIVER_H__

#define CFG_REG             (0x00)
#define SV_REG              (0x01)
#define BV_REG              (0x02)
#define CUR_REG             (0x04)
#define CAL_REG             (0x05)
#define INA226DeviceAddr    (0x40) 

#define STAMAXCURRENT		(86)	//静态电流上限,85
#define STAMINCURRENT		(19)	//静态电流下限,45
#define WORKMAXCURRENT		(1651)	//动态电流上限,1550
#define WORKMINCURRENT		(349)	//动态电流下限,1250

extern uint8_t g_Have226I2CMode;	//有I2C模块标志，在I2C中断中置1，用后清0
extern uint8_t IN226WriteDataBuff[5];
extern uint8_t IN226ReadDataBuff[5];

uint8_t ReadCurrentInfo(void);


#endif

