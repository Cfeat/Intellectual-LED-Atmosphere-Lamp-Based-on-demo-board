#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"
#include "nbiot.h"
#include "utils.h"

NB_STA nbiot_reboot(int iTimeout)
{
	/*
	REBOOTING
?@?
Boot: Unsigned
Security B..

Verified
Protocol A..

Verified
Apps A......

Verified


REBOOT_CAUSE_APPLICATION_AT
Neul
OK
*/
	NB_STA atRet = NB_TIMEOUT;
	char *AT_NRB = "AT+NRB\r\n";
	int ch = 0;//0 ok, 1 error, 2 pending
	char okFlag = 0, rebootFlag = 0;
	char *acREBOOT = "REBOOT_CAUSE_APPLICATION_AT";
	char acOK[4] = {'O','K',0x0D,0x0A};

	KE1_Send_AT(AT_NRB);

	do{
		ch = KE1_Get_NB_Ch();
		if(-1 != ch){
			//printf("%02X:%d - %d\n", ch, okFlag, errFlag);
			KE1_Put_Console_Ch((uint8_t)ch);
			if(27 != rebootFlag){
				if(acREBOOT[rebootFlag] == ch){
					rebootFlag++;
				}else{
					rebootFlag = 0;
				}
			}else{
				if(acOK[okFlag] == ch){
					okFlag++;
				}else{
					okFlag = 0;
				}
			}

			if(4 == okFlag){atRet = NB_OK; break;}
		}else{
			if(0 != iTimeout) {
				iTimeout--;
				if(0 != iTimeout){
					HAL_Delay(1000);
				}
			}
		}
	}while(iTimeout);
	return atRet;
}

NB_STA nbiot_get_signl_val(char *pcSigVal)
{
	//+CSQ:19,xx
	NB_STA atRet = NB_TIMEOUT;
	char *AT_CSQ = "AT+CSQ\r\n";
	int iTimeout = 3, ch = 0;
	char csqFlag = 0, csqOk = 0, atOK = 0, atErr = 0;
	char *acCSQ = "+CSQ:";
	char acOK[] = {'O','K',0x0D,0x0A}, acErr[] = {'E','R','R','O','R',0x0D,0x0A};
	pcSigVal[0] = 0;pcSigVal[1] = 0;

	KE1_Send_AT(AT_CSQ);HAL_Delay(500);
	do{
		ch = KE1_Get_NB_Ch();
		if(-1 != ch){
			KE1_Put_Console_Ch((uint8_t)ch);
			if(acOK[atOK] == ch){
				atOK++;
			}else{
				atOK = 0;
			}
			if(4 == atOK && 1 == csqOk){
				atRet = NB_OK; break;
			}
			if(acErr[atErr] == ch){
				atErr++;
			}else{
				atErr = 0;
			}
			if(7 == atErr){
				atRet = NB_ERROR; break;
			}
			if(1 == csqOk){
				if(',' == ch) {
					csqFlag = 0x0F;
				}
				if(3 > csqFlag){
					pcSigVal[csqFlag++] = ch;
				}
				continue;
			}
			if(acCSQ[csqFlag] == ch){
				csqFlag++;
			}else{
				csqFlag = 0;
			}
			if(5 == csqFlag){csqOk = 1; csqFlag = 0;}
		}else{
			if(0 != iTimeout) {
				iTimeout--;
				if(0 != iTimeout){
					HAL_Delay(500);
				}
			}
		}
	}while(iTimeout);
	return atRet;
}

NB_STA nbiot_check_reg(int iTimeout)
{
	/*
	AT+CGATT?

	+CGATT:1

	OK

	*/
	NB_STA atRet = NB_TIMEOUT;
	char *AT_R_CGATT = "AT+CGATT?\r\n";
	int ch = 0;// atRet = 2;//0 ok, 1 error, 2 pending
	char regFlag = 0, okFlag = 0, regSta = 0;
	char *acCGATT = "+CGATT:";
	char acOK[4] = {'O','K',0x0D,0x0A};
	KE1_Send_AT(AT_R_CGATT);HAL_Delay(1000);
	do{
		ch = KE1_Get_NB_Ch();
		if(-1 != ch){
			KE1_Put_Console_Ch((uint8_t)ch);
			if(7 == regFlag && 0 == regSta){
				regSta = ch;
				printf("\nRegister regSta:%c\n", regSta);
			}
			if(7 != regFlag){
				if(acCGATT[regFlag] == ch){
					regFlag++;
				}else{
					regFlag = 0;
				}
			}
			if(4 != okFlag){
				if(acOK[okFlag] == ch){
					okFlag++;
				}else{
					okFlag = 0;
				}
			}

			if('1' == regSta && 4 == okFlag){
				atRet = NB_OK; break;
			}
			if('0' == regSta && 4 == okFlag){
				atRet = NB_NET_CLOSED; break;
			}
			HAL_Delay(10);
		}else{
			if(0 != iTimeout) {
				HAL_Delay(1000);
				iTimeout--;
			}
		}
	}while(iTimeout);

	if(NB_OK == atRet){
		KE1_Clear_AT_Buf();
		printf("\nRegister success!\n");
	}
	return atRet;
}

NB_STA nbiot_get_nuestats(int *piRSRP, int *piSNR, int *piCellID, unsigned char *pucECL)
{
	/*
AT+NUESTATS

[2019-05-15 16:32:52.893]# RECV ASCII>

Signal power:-784
Total power:-687
TX power:-32768
TX time:0
RX time:311653
Cell ID:150360403
ECL:1
SNR:17
EARFCN:2506
PCI:495
RSRQ:-131
OPERATOR MODE:4

OK
*/
	NB_STA atRet = NB_TIMEOUT;
	int iTimeout = 3 , ch = 0, i = 0;
	char *AT_R_NUESTATS = "AT+NUESTATS\r\n";

	char *pcRSRP="Signal power:", *pcSNR = "SNR:";
	char *pcCellID = "Cell ID:", *pcECL="ECL:";
	char acVal[15] = {0};
	char acOK[] = {'O','K',0x0D,0x0A}, acErr[] = {'E','R','R','O','R',0x0D,0x0A};

	char rsrpFlag = 0, snrFlag = 0, cidFlag = 0, eclFlag = 0, atOK = 0, atErr = 0;

	*piRSRP = 0; *piSNR = 0; *piCellID = 0; *pucECL = 0;
	KE1_Clear_AT_Buf();
	KE1_Send_AT(AT_R_NUESTATS);
	HAL_Delay(3000);
	do{
		ch = KE1_Get_NB_Ch();
		if(-1 != ch){
			KE1_Put_Console_Ch((uint8_t)ch);
			if(acOK[atOK] == ch){
				atOK++;
			}else{
				atOK = 0;
			}
			if(4 == atOK){
				atRet = NB_OK; break;
			}
			if(acErr[atErr] == ch){
				atErr++;
			}else{
				atErr = 0;
			}
			if(7 == atErr){
				atRet = NB_ERROR; break;
			}
			if(13 == rsrpFlag){
				if(0x0D == ch){acVal[i] = 0; *piRSRP = atoi(acVal);  i = 0; rsrpFlag = 0; continue;}
				acVal[i++] = ch;
			}

			if(8 == cidFlag){
				if(0x0D == ch){acVal[i] = 0; *piCellID = atoi(acVal);  i = 0; cidFlag = 0; continue;}
				acVal[i++] = ch;
			}

			if(4 == eclFlag){
				if(0x0D == ch){acVal[i] = 0; *pucECL = (unsigned char)atoi(acVal);  i = 0; eclFlag = 0; continue;}
				acVal[i++] = ch;
			}

			if(4 == snrFlag){
				if(0x0D == ch){acVal[i] = 0; *piSNR = atoi(acVal);  i = 0; snrFlag = 0; continue;}
				acVal[i++] = ch;
			}
			//printf("%c-%d-%d-%d-%d\r\n", ch,rsrpFlag, cidFlag, eclFlag, snrFlag);
			if(13 != rsrpFlag){
				if(pcRSRP[rsrpFlag] == ch){
					rsrpFlag++;
				}else{
					rsrpFlag = 0;
				}
			}
			if(8 != cidFlag){
				if(pcCellID[cidFlag] == ch){
					cidFlag++;
				}else{
					cidFlag = 0;
				}
			}
			if(4 != eclFlag){
				if(pcECL[eclFlag] == ch){
					eclFlag++;
				}else{
					eclFlag = 0;
				}
			}
			if(4 != snrFlag){
				if(pcSNR[snrFlag] == ch){
					snrFlag++;
				}else{
					snrFlag = 0;
				}
			}
		}else{
			if(0 != iTimeout) {
				iTimeout--;
				if(0 != iTimeout){
					HAL_Delay(1000);
				}
			}
		}
	}while(iTimeout);
	return atRet;
}

int nbiot_parse_NNMI(char *pcNNMI, char *pcOut)
{
	char ch = 0, lenFlag = 0, dataFlag = 0;
	char acLen[5] = {0};
	int i = 0, pos = 0, dLen = 0;
	//+NNMI:6,010009020000
	for(i=0; i<strlen(pcNNMI); i++){
		ch = pcNNMI[i];
		if(0x0D == ch && 1 == dataFlag) break;

		if(':' == ch){
			lenFlag = 1; pos = 0;
			continue;
		}
		if(',' == ch){
			lenFlag = 0;
			dataFlag = 1; pos = 0;
			dLen = atoi(acLen);
			continue;
		}
		if(1 == lenFlag){
			acLen[pos++] = ch;
		}
		if(1 == dataFlag){
			pcOut[pos++] = ch;
		}
	}
	return dLen;
}

static uint8_t s_aucCmdData[128] = {0};
int nbiot_parse_ct_msg(char const *pcData, CTMessage *pstMsg)
{
	//+NNMI:12,01 008D 02 0006 313233343536
	//+NNMI:6,01 0091 04 0000
	int ret = 0, msgLen = strlen(pcData);

	vdByteString2byte(pcData, s_aucCmdData, (int *)&msgLen);
	if(6 > msgLen) return -1;

	pstMsg->headID = s_aucCmdData[0];

	if(0x01 == pstMsg->headID){
		memcpy(pstMsg->msgID, &s_aucCmdData[1], 2);
		pstMsg->userCmd = s_aucCmdData[3];
		pstMsg->valLen = (s_aucCmdData[4]<<8)|s_aucCmdData[5];

		if(0 < pstMsg->valLen && 16 >= pstMsg->valLen){
			memcpy(pstMsg->userVal, &s_aucCmdData[6], pstMsg->valLen);
		}

	}else{
		ret = -2;
	}
	return ret;
}
