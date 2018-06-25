#ifndef __UARTDRIVER_H__
#define __UARTDRIVER_H__

//���⹤����PC���ͨѶ����
#define NFC_START_CMD (0x01)
#define NFC_SEND_INFO_CMD (0x02)
#define NFC_GETVERSION_CMD (0x03)
#define NFC_GETRESETFLAG_CMD (0x04)
#define NFC_SENDCURRENT_CMD (0x05)
#define NFC_SENDREQUID_CMD	(0x06)	//����λ�������ȡ��ǩ��UID��Ϣ

#define CURRENTBUFF_LEN (5)			 /* ������Ϣ���� */
#define VERSIONBUFF_LEN (2)
#define WDTRESETBUFF_LEN (1)
#define UARTBUFFERLEND (40)

/************����ͨ������궨��***************/
#define UartNOP  0            /*  ���ڽ��մ�������*/
#define UartSOP1  1            /*  ������ʼλ*/
#define UartSOP2  2            /*  ������ʼλ*/
#define UartCMD  3            /*  ��������*/
#define UartLEN  4            /*  ���ճ���*/
#define UartDATA 5            /*  ��������*/
#define UartXOR  6            /*  ���ݳ���Ϊ0 */

extern uint8_t g_CurrentInfo[CURRENTBUFF_LEN];
extern volatile uint8_t g_UartRxOkFlag;
extern uint8_t g_UartReadBuffer[UARTBUFFERLEND];

uint8_t SendCurrentDataToPC(void);
uint8_t SendTagDataToPC(void);
uint8_t SendVersionDataToPC(void);


#endif

