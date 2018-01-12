/*
* LSxxx.cpp
*
*  Created on: 05-09-2017
*  Author: Zihong MA
***************************************************************************
*  OSIGHT *
* www.osighttech.com  *
***************************************************************************/

#include <csignal>
#include <cstdio>
#include "LSxxx.h"
#include <time.h>
#include <WinSock2.h>
#include <stdlib.h>
LSxxx                             laser;
PARA_SYNC_RSP                        g_stRealPara;
MEAS_DATA_NO_INTENSITY        g_stMeasDataNoIntensity;
MEAS_DATA_HAVE_INTENSITY1   g_stMeasDataHaveIntensity1;
MEAS_DATA_HAVE_INTENSITY2   g_stMeasDataHaveIntensity2;

extern int           server_socket_fd;
extern BITS8      g_aucRxBuf[RX_BUFSIZE];
extern BITS8      g_aucTxBuf[TX_BUFSIZE];
extern POINT0    DataIntensity0[2000];
extern POINT1    DataIntensity1[2000];
extern POINT2    DataIntensity2[2000];


/*BITS32  LSXXX(std::string hostPC, BITS32 portPC, BITS8 IntensityStatus)
{

	BITS8   err;

	//std::string hostPC="192.168.1.100";
	//BITS32  portPC=5500;
	
	while (1)
	{
		laser.connect(hostPC, portPC);
		if (!laser.isConnected())
		{
			continue;
		}
		printf("Network connection OK \r\n");


		do
		{
			err = laser.ParaSync();
		} while (0 != err);
		printf("Parameter synchronization OK\r\n");

		//g_stRealPara.ucIntensityStatus = 0;
		//g_stRealPara.ucIntensityStatus = 1;
		//g_stRealPara.ucIntensityStatus = 2;


		g_stRealPara.ucIntensityStatus = IntensityStatus;

		do
		{
			err = laser.ParaConfiguration();
		} while (0 != err);
		printf("Parameter configuration OK\r\n");

		laser.StartMeasureTransmission();


		printf("Start getting the Measurements ...\r\n");
		while (1)
		{
			err = laser.GetLidarMeasData();
			if (0 == err)
			{
				//test: Print receiving ridar data 
				for (int i = 0; i<1080; i++)
				{
					printf("DataIntensity0[ %d].ulDistance=%d\r\n", i, DataIntensity0[i].ulDistance);
					printf("DataIntensity1[ %d].ulDistance=%d\r\n", i, DataIntensity1[i].ulDistance);
					printf("DataIntensity1[ %d].ucIntensity=%d\r\n", i, DataIntensity1[i].ucIntensity);
					printf("DataIntensity2[ %d].ulDistance=%d\r\n", i, DataIntensity2[i].ulDistance);
					printf("DataIntensity2[ %d].usIntensity=%d\r\n\r\n", i, DataIntensity2[i].usIntensity);
				}

			}
			else
			{
				//break;
			}

		}

	}

	return 0;
}
*/
BITS32  LSXXX(std::string hostPC, BITS32 portPC, BITS8 IntensityStatus, BITS8 *pAlarmPara)
{

	BITS8   err;
	BITS8   i = 0;

	//std::string hostPC="192.168.1.100";
	//BITS32  portPC=5500;

	while (1)
	{
		laser.connect(hostPC, portPC);
		if (!laser.isConnected())
		{
			continue;
		}
		printf("Network connection OK \r\n");


		do
		{
			err = laser.ParaSync();
		} while (0 != err);
		printf("Parameter synchronization OK\r\n");

		//g_stRealPara.ucIntensityStatus = 0;
		//g_stRealPara.ucIntensityStatus = 1;
		//g_stRealPara.ucIntensityStatus = 2;

		g_stRealPara.ucIntensityStatus = IntensityStatus;

		//Alarm Area Para
		if ((0x00 != pAlarmPara[0]) || (0x00 != pAlarmPara[1]) || (0x00 != pAlarmPara[2]) || (0x00 != pAlarmPara[3]) || (0x00 != pAlarmPara[4]))
		{
			g_stRealPara.stAlarmArea[pAlarmPara[0]].ucAreaType = pAlarmPara[1];
			//printf("pAlarmPara[0]=%d\r\n",pAlarmPara[0]);
			//printf("g_stRealPara.stAlarmArea[pAlarmPara[0]].ucAreaType=%d\r\n",g_stRealPara.stAlarmArea[pAlarmPara[0]].ucAreaType);
			for (i = 0; i<19; i++)
			{
				g_stRealPara.stAlarmArea[pAlarmPara[0]].aucPara[i] = pAlarmPara[i + 2];
				//printf("g_stRealPara.stAlarmArea[%d].aucPara[%d]=%d\r\n",pAlarmPara[0],i,g_stRealPara.stAlarmArea[pAlarmPara[0]].aucPara[i]);
			}
		}

		do
		{
			err = laser.ParaConfiguration();
		} while (0 != err);
		printf("Parameter configuration OK\r\n");

		laser.StartMeasureTransmission();


		printf("Start getting the Measurements ...\r\n");
		while (1)
		{
			err = laser.GetLidarMeasData();
			if (0 == err)
			{
				/*test: Print receiving ridar data */
				for (int i = 0; i<1080; i++)
				{
					printf("DataIntensity0[ %d].ulDistance=%d\r\n", i, DataIntensity0[i].ulDistance);
					printf("DataIntensity1[ %d].ulDistance=%d\r\n", i, DataIntensity1[i].ulDistance);
					printf("DataIntensity1[ %d].ucIntensity=%d\r\n", i, DataIntensity1[i].ucIntensity);
					printf("DataIntensity2[ %d].ulDistance=%d\r\n", i, DataIntensity2[i].ulDistance);
					printf("DataIntensity2[ %d].usIntensity=%d\r\n\r\n", i, DataIntensity2[i].usIntensity);
				}

			}
			else
			{
				//break;
			}

		}

	}

	return 0;
}

int main(int argc, char **argv)
{
	
	//LSXXX("192.168.1.100", 5500, 0);
	// LSXXX("192.168.1.100",5500,1);
	// LSXXX("192.168.1.100",5500,2);
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return 1;
	}
	else
		printf("The Winsock 2.2 dll was found okay\n");


	/* The Winsock DLL is acceptable. Proceed to use it. */

	/* Add network programming using Winsock here */
	BITS8  i = 0;
	BITS8 aAlarmPara[21] = { 0 };


	if (((atoi(argv[1]) == atoi(argv[2]) * 2) || (atoi(argv[1]) == atoi(argv[2]) * 2 + 1)) && (atoi(argv[1])<16))
	{
		for (i = 1; i<argc; i++)
		{
			aAlarmPara[i - 1] = atoi(argv[i]);
			printf("aAlarmPara[%d]=%d\r\n", i - 1, aAlarmPara[i - 1]);
		}
	}
	else
	{
		printf("[Alarm para error] The alarm area is not corresponding to the alarm type £º%d,%d\r\n", atoi(argv[1]), atoi(argv[2]));
		return 0;
	}

	switch (aAlarmPara[1])
	{
	case 0:
	{
		if ((0x00 == aAlarmPara[2]) && (0x00 == aAlarmPara[3]) && (0x00 == aAlarmPara[4]))
		{
			printf("The alarm parameters do not change\r\n");
		}
		else if ((aAlarmPara[2] >= 0) && (aAlarmPara[2] <= 200) &&
			(aAlarmPara[3] >= 0) && (aAlarmPara[3] <= 200) &&
			(aAlarmPara[4] >= 50) && (aAlarmPara[4] <= 100))
		{
			printf("Square alarm parameter is correct   \r\n");
		}
		else
		{
			printf("[Alarm para error]Square alarm parameters:%d,%d,%d\r\n", aAlarmPara[2], aAlarmPara[3], aAlarmPara[4]);
			return 0;
		}
		break;
	}
	case 1:
	{
		if ((aAlarmPara[2] >= 0) && (aAlarmPara[2] <= 200) &&
			(aAlarmPara[3] >= 50) && (aAlarmPara[3] <= 100))
		{
			printf("The semicircle alarm parameter is correct \r\n");
		}
		else
		{
			printf("[Alarm para error]Semicircular alarm parameters:%d,%d\r\n", aAlarmPara[2], aAlarmPara[3]);
			return 0;
		}
		break;
	}
	default:
	{
		printf("Error of alarm type parameter:%d\r\n", aAlarmPara[1]);
		return 0;
	}
	}

	LSXXX("192.168.1.100", 5500, 0, (BITS8 *)aAlarmPara);

	//LSXXX("192.168.1.100", 5500, 0);

	/* then call WSACleanup when done using the Winsock dll */

	WSACleanup();

}
