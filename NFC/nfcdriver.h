#ifndef __NFCDRIVER_H__
#define __NFCDRIVER_H__

#define I2CRECVDATAFROMNFCLEN (21)	/* I2C���յ����ݵĳ��� */
#define TOTAL_TAG_NUM   (4)

#define I2CReadVersionCMD (4)
#define I2CReadResetFlagCMD (5)
#define I2CReadWorkCurrentCMD (6)

//���峣��
#define OK (0)
#define ERROR (1)
#define NFCModeDeviceAddr (0x7E)		/* NFCģ���I2C��ַ*/
#define I2CSENDDATATONFCLEN (6)		/* I2C��Ҫ���͵����ݳ��� */

extern uint8_t g_TagInfo[TOTAL_TAG_NUM][I2CRECVDATAFROMNFCLEN];
extern uint8_t g_VersionInfo[I2CRECVDATAFROMNFCLEN];
extern uint8_t g_NFCResetInfo[I2CRECVDATAFROMNFCLEN];

extern uint8_t g_HaveI2CMode;	//��I2Cģ���־����I2C�ж�����1���ú���0
extern uint8_t g_I2CRecvDataBuffer[I2CRECVDATAFROMNFCLEN];
extern volatile uint8_t  g_FeedDogFlag;

uint8_t ReadNFCTagInfo(void);
uint8_t ReadNFCVersion(void);
uint8_t ReadNFCResetFlag(void);
uint8_t ReadDataFormNFCMode(uint8_t id);


#endif

