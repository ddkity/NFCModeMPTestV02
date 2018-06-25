#ifndef __INA226DRIVER_H__
#define __INA226DRIVER_H__

#define CFG_REG             (0x00)
#define SV_REG              (0x01)
#define BV_REG              (0x02)
#define CUR_REG             (0x04)
#define CAL_REG             (0x05)
#define INA226DeviceAddr    (0x40) 

#define STAMAXCURRENT		(86)	//��̬��������,85
#define STAMINCURRENT		(19)	//��̬��������,45
#define WORKMAXCURRENT		(1651)	//��̬��������,1550
#define WORKMINCURRENT		(349)	//��̬��������,1250

extern uint8_t g_Have226I2CMode;	//��I2Cģ���־����I2C�ж�����1���ú���0
extern uint8_t IN226WriteDataBuff[5];
extern uint8_t IN226ReadDataBuff[5];

uint8_t ReadCurrentInfo(void);


#endif

