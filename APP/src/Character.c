#include <ctype.h>
#include "string.h"
#include "esp_log.h"

static const char *TAG = "CHARACTER";

char  AsciiToHex(char * pAscii,  char * pHex, int nLen)
{
	int nHexLen = nLen / 2;
       char Nibble[2] = {0};
	int i = 0;
	int j = 0;

	// if (nLen%2)
	// {
	// 	return 1;
	// }

	for (i = 0; i < nHexLen; i ++)
	{
		Nibble[0] = *pAscii ++;		
		Nibble[1] = *pAscii ++;
		for (j = 0; j < 2; j ++)
		{
			if (Nibble[j] <= 'F' && Nibble[j] >= 'A')
				Nibble[j] = Nibble[j] - 'A' + 10;
			else if (Nibble[j] <= 'f' && Nibble[j] >= 'a')
				Nibble[j] = Nibble[j] - 'a' + 10;
			else if (Nibble[j] >= '0' && Nibble[j] <= '9')
				Nibble [j] = Nibble[j] - '0';
			else
				return 1;//Nibble[j] = Nibble[j] - 'a' + 10;
			
		}	// for (int j = ...)
		pHex[i] = Nibble[0] << 4;	// Set the high nibble
		pHex[i] |= Nibble[1];	//Set the low nibble
	}	// for (int i = ...)
	return 0;
}

int HexToAscii( char *pHexStr, char *pAscStr,int Len)
{
	char Nibble[2];
	char Buffer[2048];
	int i = 0;
	int j = 0;

	for(i=0;i<Len;i++)
	{
		Nibble[0]=pHexStr[i] >> 4 & 0X0F;
		Nibble[1]=pHexStr[i] & 0x0F;
		for(j=0;j<2;j++)
		{
			if(Nibble[j]<10)
			{
				Nibble[j]=Nibble[j]+0x30;
			}
			else if(Nibble[j]<16)
			{
				Nibble[j]=Nibble[j]-10+'A';
			}
			else
			{
				return 0;
			}
		}
		memcpy(Buffer+i*2,Nibble,2);
	}
	Buffer[2*Len]=0x00;
	memcpy(pAscStr,Buffer,2*Len);
	pAscStr[2*Len]=0x00;
	return 1;
}

// void AsciiToHex(char *src, uint8_t *dest, int len)
// {
//     int dh,dl;   // 16进制的高4位和低4位
//     char ch,cl;  // 字符串的高位和低位
//     int i;
//     if(src == NULL || dest == NULL)
//     {
//        return;
//     }
//     if(len < 1)
//     {
//        return;
//     }
//     for(i = 0; i < len; i++)
//     {
//        //ESP_LOGI(TAG, "loop");   
//        ch = src[2*i];
//        cl = src[2*i+1];
//        dh = toupper(ch) - '0';
//        if(dh > 9)
//        {
//               dh = toupper(ch) - 'A' + 10;
//               //ESP_LOGI(TAG, "if(dh > 9)");   
//        }
//        dl = toupper(cl) - '0';
//        if(dl > 9)
//        {
//               dl = toupper(cl) - 'A' + 10;   
//               //ESP_LOGI(TAG, "if(dl > 9)");   
//        }
//        dest[i] = dh * 16 + dl;
//     }
//     if(len%2 > 0)//字符串个数为奇数
//     {
//        ESP_LOGI(TAG, "13579……");
//        dest[len/2] = src[len-1] - '0';
//        if(dest[len/2] > 9)
//        {
//               dest[len/2] = toupper(src[len-1]) - 'A' + 10;
//        }
//     }
// }
// void HexToAscii(uint8_t *src, char *dest, int len)
// {
//        char dh,dl;  //字符串的高位和低位
//        int i;
//        if(src == NULL || dest == NULL)
//        {
//               //printf("src or dest is NULL\n");
//               return;
//        }
//        if(len < 1)
//        {
//               //printf("length is NULL\n");
//               return;
//        }
//        for(i = 0; i < len; i++)
//        {
//               dh = '0' + src[i] / 16;
//               dl = '0' + src[i] % 16;
//               if(dh > '9')
//               {
//                      dh = dh - '9' - 1 + 'A'; // 或者 dh= dh+ 7;
//               }
//               if(dl > '9')
//               {
//                      dl = dl - '9' - 1 + 'A'; // 或者dl = dl + 7;
//               }
//               dest[2*i] = dh;
//               dest[2*i+1] = dl;
//        }
//        dest[2*i] = '\0';
// }