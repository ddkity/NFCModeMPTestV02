#ifndef __UARTDRIVER_H__
#define __UARTDRIVER_H__

//产测工具与PC间的通讯命令
#define NFC_START_CMD (0x01)
#define NFC_SEND_INFO_CMD (0x02)
#define NFC_GETVERSION_CMD (0x03)
#define NFC_GETRESETFLAG_CMD (0x04)
#define NFC_SENDCURRENT_CMD (0x05)
#define NFC_SENDREQUID_CMD	(0x06)	//向上位机请求获取标签的UID信息

#define CURRENTBUFF_LEN (5)			 /* 电流信息长度 */
#define VERSIONBUFF_LEN (2)
#define WDTRESETBUFF_LEN (1)
#define UARTBUFFERLEND (40)

/************串口通信命令宏定义***************/
#define UartNOP  0            /*  串口接收错误或空闲*/
#define UartSOP1  1            /*  接收起始位*/
#define UartSOP2  2            /*  接收起始位*/
#define UartCMD  3            /*  接收命令*/
#define UartLEN  4            /*  接收长度*/
#define UartDATA 5            /*  接收数据*/
#define UartXOR  6            /*  数据长度为0 */

extern uint8_t g_CurrentInfo[CURRENTBUFF_LEN];
extern volatile uint8_t g_UartRxOkFlag;
extern uint8_t g_UartReadBuffer[UARTBUFFERLEND];

uint8_t SendCurrentDataToPC(void);
uint8_t SendTagDataToPC(void);
uint8_t SendVersionDataToPC(void);


#endif

