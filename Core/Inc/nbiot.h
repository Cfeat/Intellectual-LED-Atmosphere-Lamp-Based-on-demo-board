#ifndef __NB_IOT_H_
#define __NB_IOT_H_

typedef enum{
	NB_OK = 0,
	NB_TIMEOUT = 1,
	NB_ERROR = 2,
	NB_DATA_IN = 3,
	NB_NET_CLOSED = 4,
	NB_REG_CHANGED = 5
}NB_STA;

typedef enum{
	NB_STEP_BUFF_CLEAR = 0,
	NB_STEP_CHECK_AT,
	NB_STEP_CHECK_REG,
	NB_STEP_STOP_MODULE,
	NB_STEP_SET_COAP,
	NB_STEP_START_MODULE,
	NB_STEP_SET_PDP,
	NB_STEP_SIM_CHECK,
	NB_STEP_START_REG,
	NB_STEP_SET_AUTO_REG,
	NB_STEP_WAITING_REG_OK,
	NB_STEP_UP_REG_INFO,
	NB_STEP_UP_DEV_INFO
}NB_WORK_STEP;

typedef enum MSG_CMD_t{
	CTL_LED = 0x01,
	CTL_LCD = 0x02,
	CTL_SWITCH_1 = 0x03,
	CTL_SWITCH_2 = 0x04,

	MSG_ACK = 0x30
}MSG_CMD;

typedef struct ct_message_t{//电信物联网云平台数据报文格式
	//+NNMI:12,01 008D 02 0006 313233343536
	uint8_t headID;
	uint8_t msgID[2];
	uint8_t userCmd;
	uint16_t valLen;
	uint8_t userVal[17];
}CTMessage;

NB_STA nbiot_reboot(int iTimeout);
NB_STA nbiot_check_reg(int iTimeout);
NB_STA nbiot_get_signl_val(char *pcSigVal);
NB_STA nbiot_get_nuestats(int *piRSRP, int *piSNR, int *piCellID, unsigned char *pucECL);
int nbiot_parse_NNMI(char *pcNNMI, char *pcOut);
int nbiot_parse_ct_msg(char const *pcData, CTMessage *pstMsg);
#endif
