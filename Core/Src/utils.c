#include <string.h>
#include <stdio.h>
#include <stdlib.h>
void vdAscii2hex(char *pcAscii, char *pcHex)
{
	int iDlen = 0, i = 0, pos = 0;
	iDlen = strlen(pcAscii);
	if(128 < iDlen) return;
	for(i=0; i<iDlen; i++){
		sprintf(&pcHex[pos], "%02X", pcAscii[i]);
		pos += 2;
	}
}

void vdByteString2byte(char const *i_szAsc, unsigned char *ppbt_Byte, int *iByteLength)
{
	int iLength = *iByteLength;
	int iPos = 0;
	int iLeft = 0;
	int iRight = 0;
	int iIndex = 0;

	for (iIndex = 0; iIndex < iLength; iIndex++, iPos++){
		iLeft = i_szAsc[iIndex++];
		iRight = i_szAsc[iIndex];

		iLeft = (iLeft < 58) ? (iLeft - 48) : (iLeft - 55);
		iRight = (iRight < 58) ? (iRight - 48) : (iRight - 55);

		ppbt_Byte[iPos] = ((iLeft << 4) | iRight)& 0xff;
	}

	*iByteLength = iPos;

	return;
}
