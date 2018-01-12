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


BITS32  LSXXX(std::string hostPC, BITS32 portPC, BITS8 IntensityStatus)
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
	LSXXX("192.168.1.100", 5500, 0);

	/* then call WSACleanup when done using the Winsock dll */

	WSACleanup();

}
