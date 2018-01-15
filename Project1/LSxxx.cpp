/*
* LSxxx.cpp
*
*  Created on: 05-09-2017
*  Author: Zihong Ma
***************************************************************************
*  OSIGHT *
* www.osighttech.com  *
***************************************************************************/

#include <time.h>
#include <sys/types.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
//#include <unistd.h>
#include <io.h>
#include "LSxxx.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wsock32.lib")

#define SERVER_PORT 5500
#define CLIENT_PORT 6500    

struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int server_socket_fd;
socklen_t client_addr_length;
socklen_t server_addr_length;

BITS8      g_aucRxBuf[RX_BUFSIZE];
BITS8      g_aucTxBuf[TX_BUFSIZE];

POINT0    DataIntensity0[2000];
POINT1    DataIntensity1[2000];
POINT2    DataIntensity2[2000];



extern LSxxx laser;
extern PARA_SYNC_RSP            g_stRealPara;
extern MEAS_DATA_NO_INTENSITY      g_stMeasDataNoIntensity;
extern MEAS_DATA_HAVE_INTENSITY1   g_stMeasDataHaveIntensity1;
extern MEAS_DATA_HAVE_INTENSITY2   g_stMeasDataHaveIntensity2;

LSxxx::LSxxx() : connected_(false)
{
}

LSxxx::~LSxxx()
{
}

INT32 LSxxx::connect(std::string hostPC, int portPC)
{
	/* UDP */
	/* server  */
//	bzero(&server_addr, sizeof(server_addr));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, hostPC.c_str(), &server_addr.sin_addr);
	server_addr.sin_port = htons(portPC);

	/* client */
	//bzero(&client_addr, sizeof(client_addr));  
	//client_addr.sin_family = AF_INET;  
	//inet_pton(AF_INET, host.c_str(), &client_addr.sin_addr);
	//client_addr.sin_port = htons(port);  

	/* create socket */
	server_socket_fd = -1;
	server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_socket_fd < 0)
	{
		perror("Create Socket Failed:");
		return 1;
	}

	/* setsockopt */
	const char on = 1;
	if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))<0)
	{
		perror(" setsockopt error ");
		return 1;
	}

	/* bind */
	if (-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
	{
		perror("Server Bind Failed:");
		return 1;
	}

	client_addr_length = sizeof(client_addr);
	server_addr_length = sizeof(server_addr);

	connected_ = true;

	return 0;
}


#if 0
void LSxxx::connect(std::string host, int port)
{
	/* tcp */
	if (!connected_)
	{
		logDebug("Creating non-blocking socket.");
		socket_fd_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_fd_)
		{
			struct sockaddr_in stSockAddr;
			stSockAddr.sin_family = PF_INET;
			stSockAddr.sin_port = htons(port);
			inet_pton(AF_INET, host.c_str(), &stSockAddr.sin_addr);

			logDebug("Connecting socket to laser.");
			int ret = ::connect(socket_fd_, (struct sockaddr *) &stSockAddr, sizeof(stSockAddr));

			if (ret == 0)
			{
				connected_ = true;
				logDebug("Connected succeeded.");
			}
		}
	}
}
#endif

void LSxxx::disconnect()
{
	if (connected_)
	{
		_close(socket_fd_);
		connected_ = false;
	}
}

bool LSxxx::isConnected()
{
	return connected_;
}

INT32 LSxxx::ParaSync()
{
	BITS8  ucCount = 30;
	INT32  ulSendLength = 0;
	BITS32 ulMsgId = NULL_MSGID;
	PARA_SYNC_REQ  stParaSynReq;
	PARA_SYNC_RSP  stParaSynRsp;

	stParaSynReq.ulMsgId = PARA_SYNC_REQ_ID;
	stParaSynReq.usTransId = 0x0505;
	printf("Parameter synchronization start\r\n");
	memset(g_aucTxBuf, 0, sizeof(g_aucTxBuf));
	ulSendLength = laser.PackParaSyncReq(&stParaSynReq, g_aucTxBuf);

	if (ulSendLength > VALUE_0)
	{
		laser.send_data(g_aucTxBuf, ulSendLength);
	}

	memset((BITS8*)&stParaSynRsp, 0, sizeof(PARA_SYNC_RSP));
	memset((BITS8*)&g_stRealPara, 0, sizeof(PARA_SYNC_RSP));

	ulSendLength = 0;
	while (ucCount--)
	{
		ulSendLength = laser.read_data(g_aucRxBuf, sizeof(stParaSynRsp));
		if (ulSendLength > VALUE_0)
		{
			ulMsgId = NULL_MSGID;
			ulMsgId = (((BITS32)g_aucRxBuf[BIT_0] << OFFSET_24) | ((BITS32)g_aucRxBuf[BIT_1] << OFFSET_16) |
				((BITS32)g_aucRxBuf[BIT_2] << OFFSET_08) | ((BITS32)g_aucRxBuf[BIT_3]));

			if (PARA_SYNC_RSP_ID == ulMsgId)
			{
				laser.UnpackParaSyncRsp((BITS8 *)&g_aucRxBuf, (PARA_SYNC_RSP *)&stParaSynRsp);
				memcpy((BITS8*)&g_stRealPara, (BITS8*)&stParaSynRsp, sizeof(PARA_SYNC_RSP));

				return BUSPRO_OK;
				printf("Parameter synchronization start1\r\n");
			}
			else
			{
				return BUSPRO_ERROR;
				printf("Parameter synchronization start2\r\n");
			}
		}
	}
	return BUSPRO_ERROR;

}

INT32 LSxxx::ParaConfiguration()
{
	BITS8  ucIndex = 0;
	BITS8  ucIndex1 = 0;
	BITS8  ucCount = 30;
	INT32  ulSendLength = 0;
	BITS32 ulMsgId = NULL_MSGID;
	PARA_CONFIGURATION_REQ stParaConfiguartionReq;
	PARA_CONFIGURATION_RSP  stParaConfiguartionRsq;

	stParaConfiguartionReq.ulMsgId = PARA_CONFIGURATION_REQ_ID;
	stParaConfiguartionReq.usTransId = 0x0505;
	memset((BITS8*)&stParaConfiguartionReq.aucReserved, 0, sizeof(stParaConfiguartionReq.aucReserved));
	stParaConfiguartionReq.ucCurrentSpeed = g_stRealPara.ucCurrentSpeed;
	stParaConfiguartionReq.ucIntensityStatus = g_stRealPara.ucIntensityStatus;
	stParaConfiguartionReq.ucCurrentAreaId = g_stRealPara.ucCurrentAreaId;
	stParaConfiguartionReq.ulAngularResolution = g_stRealPara.ulAngularResolution;

	for (ucIndex = 0; ucIndex<16; ucIndex++)
	{
		stParaConfiguartionReq.stAlarmArea[ucIndex].ucAreaType = g_stRealPara.stAlarmArea[ucIndex].ucAreaType;
		for (ucIndex1 = 0; ucIndex1<19; ucIndex1++)
		{
			stParaConfiguartionReq.stAlarmArea[ucIndex].aucPara[ucIndex1] = g_stRealPara.stAlarmArea[ucIndex].aucPara[ucIndex1];
		}
	}

	memset(g_aucTxBuf, 0, sizeof(g_aucTxBuf));
	ulSendLength = laser.PackParaConfigurationReq(&stParaConfiguartionReq, g_aucTxBuf);

	if (ulSendLength > VALUE_0)
	{
		laser.send_data(g_aucTxBuf, ulSendLength);
	}

	ulSendLength = 0;

	while (ucCount--)
	{
		ulSendLength = laser.read_data(g_aucRxBuf, sizeof(stParaConfiguartionRsq));
		if (ulSendLength > VALUE_0)
		{
			ulMsgId = (((BITS32)g_aucRxBuf[BIT_0] << OFFSET_24) | ((BITS32)g_aucRxBuf[BIT_1] << OFFSET_16) |
				((BITS32)g_aucRxBuf[BIT_2] << OFFSET_08) | ((BITS32)g_aucRxBuf[BIT_3]));

			if (PARA_CONFIGURATION_RSP_ID == ulMsgId)
			{
				laser.UnpackParaConfigurationRsp((BITS8 *)&g_aucRxBuf, (PARA_CONFIGURATION_RSP *)&stParaConfiguartionRsq);
				if (0 == stParaConfiguartionRsq.ucResult)
				{
					return BUSPRO_OK;
				}
				else
				{
					return BUSPRO_ERROR;
				}
			}
			else
			{
				continue;
				//return BUSPRO_ERROR;
			}
		}
		else
		{
			//
		}
	}
	return BUSPRO_ERROR;
}

void LSxxx::StartMeasureTransmission()
{
	INT32  ulSendLength = 0;
	START_MEASURE_TRANSMISSION_REQ  stStartMeasureTransmissionReq;

	stStartMeasureTransmissionReq.ulMsgId = START_MEASURE_TRANSMISSION_ID;
	stStartMeasureTransmissionReq.ucStart = 0x01;

	memset(g_aucTxBuf, 0, sizeof(g_aucTxBuf));
	ulSendLength = laser.PackStartMeasureTransmissionReq(&stStartMeasureTransmissionReq, g_aucTxBuf);

	if (ulSendLength > VALUE_0)
	{
		laser.send_data(g_aucTxBuf, ulSendLength);
	}

}


INT32 LSxxx::GetLidarMeasData()
{
	BITS32  ulIndex = 0;
	INT32  ulSendLength = 0;
	BITS32 ulMsgId = NULL_MSGID;

	while (1)
	{
		memset(g_aucRxBuf, 0, sizeof(g_aucRxBuf));
		ulSendLength = laser.read_data(g_aucRxBuf, sizeof(g_aucRxBuf));
		if (ulSendLength > VALUE_0)
		{
			ulMsgId = (((BITS32)g_aucRxBuf[BIT_0] << OFFSET_24) | ((BITS32)g_aucRxBuf[BIT_1] << OFFSET_16) |
				((BITS32)g_aucRxBuf[BIT_2] << OFFSET_08) | ((BITS32)g_aucRxBuf[BIT_3]));

			if (MEAS_DATA_PACKAGE_ID == ulMsgId)
			{

				if (0x00 == g_stRealPara.ucIntensityStatus)
				{
					laser.UnpackMeasDataNoIntensity((BITS8 *)&g_aucRxBuf, (MEAS_DATA_NO_INTENSITY *)&g_stMeasDataNoIntensity);
					for (ulIndex = 0; ulIndex<g_stMeasDataNoIntensity.usPackMeasPointNum; ulIndex++)
					{
						DataIntensity0[g_stMeasDataNoIntensity.ucCurrentPackNO*g_stMeasDataNoIntensity.usPackMeasPointNum + ulIndex].ulDistance = g_stMeasDataNoIntensity.astPoint0[ulIndex].ulDistance;
						//printf("Get point0 measurement");
						printf("DataIntensity0.ulOutputStatus=%06x\r\n\r\n", g_stMeasDataNoIntensity.ulOutputStatus);
					}
					if ((g_stMeasDataNoIntensity.ucCurrentPackNO + 1) == g_stMeasDataNoIntensity.ucTotalPackNum)
					{
						return BUSPRO_OK;
					}
				}
				else if (0x01 == g_stRealPara.ucIntensityStatus)
				{
					laser.UnpackMeasDataHaveIntensity1((BITS8 *)&g_aucRxBuf, (MEAS_DATA_HAVE_INTENSITY1 *)&g_stMeasDataHaveIntensity1);
					for (ulIndex = 0; ulIndex<g_stMeasDataHaveIntensity1.usPackMeasPointNum; ulIndex++)
					{
						DataIntensity1[g_stMeasDataHaveIntensity1.ucCurrentPackNO*g_stMeasDataHaveIntensity1.usPackMeasPointNum + ulIndex].ulDistance = g_stMeasDataHaveIntensity1.astPoint1[ulIndex].ulDistance;
						DataIntensity1[g_stMeasDataHaveIntensity1.ucCurrentPackNO*g_stMeasDataHaveIntensity1.usPackMeasPointNum + ulIndex].ucIntensity = g_stMeasDataHaveIntensity1.astPoint1[ulIndex].ucIntensity;
						DataIntensity1[g_stMeasDataHaveIntensity1.ucCurrentPackNO*g_stMeasDataHaveIntensity1.usPackMeasPointNum + ulIndex].ulOutputStatus = g_stMeasDataHaveIntensity1.ulOutputStatus;
						printf("Get point1 measurement");
						printf("DataIntensity1.ulOutputStatus=%04x\r\n\r\n", g_stMeasDataHaveIntensity1.ulOutputStatus);
					}

					if ((g_stMeasDataHaveIntensity1.ucCurrentPackNO + 1) == g_stMeasDataHaveIntensity1.ucTotalPackNum)
					{
						return BUSPRO_OK;
					}
				}
				else if (0x02 == g_stRealPara.ucIntensityStatus)
				{
					laser.UnpackMeasDataHaveIntensity2((BITS8 *)&g_aucRxBuf, (MEAS_DATA_HAVE_INTENSITY2 *)&g_stMeasDataHaveIntensity2);
					for (ulIndex = 0; ulIndex<g_stMeasDataHaveIntensity2.usPackMeasPointNum; ulIndex++)
					{
						DataIntensity2[g_stMeasDataHaveIntensity2.ucCurrentPackNO*g_stMeasDataHaveIntensity2.usPackMeasPointNum + ulIndex].ulDistance = g_stMeasDataHaveIntensity2.astPoint2[ulIndex].ulDistance;
						DataIntensity2[g_stMeasDataHaveIntensity2.ucCurrentPackNO*g_stMeasDataHaveIntensity2.usPackMeasPointNum + ulIndex].usIntensity = g_stMeasDataHaveIntensity2.astPoint2[ulIndex].usIntensity;
				//		DataIntensity2[g_stMeasDataHaveIntensity2.ucCurrentPackNO*g_stMeasDataHaveIntensity2.usPackMeasPointNum + ulIndex].ulOutputStatus = g_stMeasDataHaveIntensity2.astPoint2[ulIndex].ulOutputStatus;
					}

					if ((g_stMeasDataHaveIntensity2.ucCurrentPackNO + 1) == g_stMeasDataHaveIntensity2.ucTotalPackNum)
					{
						return BUSPRO_OK;
					}
				}
			}
			else
			{
				//return BUSPRO_ERROR;
			}
		}
	}
	return BUSPRO_ERROR;

}



INT32 LSxxx::PackParaSyncReq(PARA_SYNC_REQ *vpstParaSyncReq, BITS8 *vpucBuff)
{
	BITS8 *pucSendNum = vpucBuff;

	if ((NULL == vpstParaSyncReq) || (NULL == vpucBuff))
	{
		return BUSPRO_ERROR_POINTER;
	}

	PACK_4_BYTE(vpucBuff, vpstParaSyncReq->ulMsgId);
	PACK_2_BYTE(vpucBuff, vpstParaSyncReq->usTransId);

	return (vpucBuff - pucSendNum);
}

INT32 LSxxx::PackParaConfigurationReq(PARA_CONFIGURATION_REQ *vpstParaConfigurationReq, BITS8 *vpucBuff)
{
	BITS8 *pucSendNum = vpucBuff;
	BITS8  ucIndex = 0;
	BITS8  ucIndex1 = 0;

	if ((NULL == vpstParaConfigurationReq) || (NULL == vpucBuff))
	{
		return BUSPRO_ERROR_POINTER;
	}

	PACK_4_BYTE(vpucBuff, vpstParaConfigurationReq->ulMsgId);
	PACK_2_BYTE(vpucBuff, vpstParaConfigurationReq->usTransId);
	for (ucIndex = 0; ucIndex<sizeof(vpstParaConfigurationReq->aucReserved); ucIndex++)
	{
		PACK_1_BYTE(vpucBuff, vpstParaConfigurationReq->aucReserved[ucIndex]);
	}
	PACK_1_BYTE(vpucBuff, vpstParaConfigurationReq->ucCurrentSpeed);
	PACK_1_BYTE(vpucBuff, vpstParaConfigurationReq->ucIntensityStatus);
	PACK_1_BYTE(vpucBuff, vpstParaConfigurationReq->ucCurrentAreaId);
	PACK_4_BYTE(vpucBuff, vpstParaConfigurationReq->ulAngularResolution);

	for (ucIndex = 0; ucIndex<16; ucIndex++)
	{
		PACK_1_BYTE(vpucBuff, vpstParaConfigurationReq->stAlarmArea[ucIndex].ucAreaType);
		for (ucIndex1 = 0; ucIndex1<19; ucIndex1++)
		{
			PACK_1_BYTE(vpucBuff, vpstParaConfigurationReq->stAlarmArea[ucIndex].aucPara[ucIndex1]);
		}
	}

	return (vpucBuff - pucSendNum);
}

INT32 LSxxx::PackStartMeasureTransmissionReq(START_MEASURE_TRANSMISSION_REQ *vpstStartMeasureTransmissionReq, BITS8 *vpucBuff)
{
	BITS8 *pucSendNum = vpucBuff;

	if ((NULL == vpstStartMeasureTransmissionReq) || (NULL == vpucBuff))
	{
		return BUSPRO_ERROR_POINTER;
	}

	PACK_4_BYTE(vpucBuff, vpstStartMeasureTransmissionReq->ulMsgId);
	PACK_1_BYTE(vpucBuff, vpstStartMeasureTransmissionReq->ucStart);

	return (vpucBuff - pucSendNum);
}

INT32 LSxxx::UnpackParaSyncRsp(BITS8 *vpucMsg, PARA_SYNC_RSP *vpstParaSyncRsp)
{
	BITS8  ucIndex = 0;
	BITS8  ucIndex1 = 0;

	if ((NULL == vpucMsg) || (NULL == vpstParaSyncRsp))
	{
		return BUSPRO_ERROR_POINTER;
	}

	UNPACK_4_BYTE(vpucMsg, vpstParaSyncRsp->ulMsgId);
	UNPACK_2_BYTE(vpucMsg, vpstParaSyncRsp->usTransId);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucMAC[0]);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucMAC[1]);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucMAC[2]);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucMAC[3]);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucMAC[4]);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucMAC[5]);
	UNPACK_4_BYTE(vpucMsg, vpstParaSyncRsp->ulSerialNum1);
	UNPACK_4_BYTE(vpucMsg, vpstParaSyncRsp->ulSerialNum2);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucDevNum);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucSoftwareVersion);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucIndex);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucLineNum);
	UNPACK_4_BYTE(vpucMsg, vpstParaSyncRsp->ulStartAngle);
	UNPACK_2_BYTE(vpucMsg, vpstParaSyncRsp->usVerticalAngle);
	UNPACK_2_BYTE(vpucMsg, vpstParaSyncRsp->usMaxDistance);
	UNPACK_4_BYTE(vpucMsg, vpstParaSyncRsp->ulPointNum);

	for (ucIndex = 0; ucIndex<sizeof(vpstParaSyncRsp->aucReserved); ucIndex++)
	{
		UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->aucReserved[ucIndex]);
	}

	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucCurrentSpeed);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucIntensityStatus);
	UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->ucCurrentAreaId);
	UNPACK_4_BYTE(vpucMsg, vpstParaSyncRsp->ulAngularResolution);

	for (ucIndex = 0; ucIndex<16; ucIndex++)
	{
		UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->stAlarmArea[ucIndex].ucAreaType);
		for (ucIndex1 = 0; ucIndex1<19; ucIndex1++)
		{
			UNPACK_1_BYTE(vpucMsg, vpstParaSyncRsp->stAlarmArea[ucIndex].aucPara[ucIndex1]);
		}
	}

	return BUSPRO_OK;
}

INT32 LSxxx::UnpackParaConfigurationRsp(BITS8 *vpucMsg, PARA_CONFIGURATION_RSP *vpstParaConfigurationRsp)
{
	if ((NULL == vpucMsg) || (NULL == vpstParaConfigurationRsp))
	{
		return BUSPRO_ERROR_POINTER;
	}

	UNPACK_4_BYTE(vpucMsg, vpstParaConfigurationRsp->ulMsgId);
	UNPACK_2_BYTE(vpucMsg, vpstParaConfigurationRsp->usTransId);
	UNPACK_1_BYTE(vpucMsg, vpstParaConfigurationRsp->ucResult);

	return BUSPRO_OK;
}

INT32 LSxxx::UnpackMeasDataNoIntensity(BITS8 *vpucMsg, MEAS_DATA_NO_INTENSITY *vpstMeasDataNoIntensity)
{
	BITS32  ulIndex = 0;

	if ((NULL == vpucMsg) || (NULL == vpstMeasDataNoIntensity))
	{
		return BUSPRO_ERROR_POINTER;
	}

	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulMsgId);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucDevNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucSoftwareVersion);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucLineNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucEcho);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulSerialNum1);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulSerialNum2);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucIntensityStatus);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucDevStatus);
	UNPACK_2_BYTE(vpucMsg, vpstMeasDataNoIntensity->usScanCounter);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulTime);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulInputStatus);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulOutputStatus);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulStartAngle);
	UNPACK_2_BYTE(vpucMsg, vpstMeasDataNoIntensity->usVerticalAngle);

	for (ulIndex = 0; ulIndex<sizeof(vpstMeasDataNoIntensity->aucReserved); ulIndex++)
	{
		UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->aucReserved[ulIndex]);
	}

	UNPACK_2_BYTE(vpucMsg, vpstMeasDataNoIntensity->usPackMeasPointNum);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->ulAngularResolution);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucTotalPackNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataNoIntensity->ucCurrentPackNO);

	for (ulIndex = 0; ulIndex<vpstMeasDataNoIntensity->usPackMeasPointNum; ulIndex++)
	{
		UNPACK_4_BYTE(vpucMsg, vpstMeasDataNoIntensity->astPoint0[ulIndex].ulDistance);
	}
	return BUSPRO_OK;
}


INT32 LSxxx::UnpackMeasDataHaveIntensity1(BITS8 *vpucMsg, MEAS_DATA_HAVE_INTENSITY1 *vpstMeasDataHaveIntensity1)
{
	BITS32  ulIndex = 0;

	if ((NULL == vpucMsg) || (NULL == vpstMeasDataHaveIntensity1))
	{
		return BUSPRO_ERROR_POINTER;
	}

	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulMsgId);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucDevNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucSoftwareVersion);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucLineNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucEcho);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulSerialNum1);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulSerialNum2);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucIntensityStatus);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucDevStatus);
	UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->usScanCounter);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulTime);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulInputStatus);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulOutputStatus);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulStartAngle);
	UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->usVerticalAngle);

	for (ulIndex = 0; ulIndex<sizeof(vpstMeasDataHaveIntensity1->aucReserved); ulIndex++)
	{
		UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->aucReserved[ulIndex]);
	}

	UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->usPackMeasPointNum);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ulAngularResolution);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucTotalPackNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->ucCurrentPackNO);

	for (ulIndex = 0; ulIndex<vpstMeasDataHaveIntensity1->usPackMeasPointNum; ulIndex++)
	{
		UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->astPoint1[ulIndex].ulDistance);
		UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity1->astPoint1[ulIndex].ucIntensity);
	}
	return BUSPRO_OK;
}


INT32 LSxxx::UnpackMeasDataHaveIntensity2(BITS8 *vpucMsg, MEAS_DATA_HAVE_INTENSITY2 *vpstMeasDataHaveIntensity2)
{
	BITS32  ulIndex = 0;

	if ((NULL == vpucMsg) || (NULL == vpstMeasDataHaveIntensity2))
	{
		return BUSPRO_ERROR_POINTER;
	}

	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulMsgId);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucDevNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucSoftwareVersion);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucLineNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucEcho);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulSerialNum1);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulSerialNum2);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucIntensityStatus);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucDevStatus);
	UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->usScanCounter);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulTime);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulInputStatus);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulOutputStatus);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulStartAngle);
	UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->usVerticalAngle);

	for (ulIndex = 0; ulIndex<sizeof(vpstMeasDataHaveIntensity2->aucReserved); ulIndex++)
	{
		UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->aucReserved[ulIndex]);
	}

	UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->usPackMeasPointNum);
	UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ulAngularResolution);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucTotalPackNum);
	UNPACK_1_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->ucCurrentPackNO);

	for (ulIndex = 0; ulIndex<vpstMeasDataHaveIntensity2->usPackMeasPointNum; ulIndex++)
	{
		UNPACK_4_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->astPoint2[ulIndex].ulDistance);
		UNPACK_2_BYTE(vpucMsg, vpstMeasDataHaveIntensity2->astPoint2[ulIndex].usIntensity);
	}
	return BUSPRO_OK;
}


void LSxxx::send_data(void* vpSrc, BITS16 usCnt)
{
	/* tcp */
	//write(socket_fd_, vpSrc, usCnt);

	/* udp */
	sendto(server_socket_fd, (const char*)vpSrc, usCnt, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
}

/**********************************************************
* 函数功能: 读取数据
***********************************************************/
INT32 LSxxx::read_data(void* vpSrc, BITS16 usCnt)
{
	/* tcp */
	//return read(socket_fd_, vpSrc, usCnt);

	/* udp */
	//return recvfrom(server_socket_fd, vpSrc, usCnt, 0, (struct sockaddr*)&client_addr, &client_addr_length);
	return recvfrom(server_socket_fd, (char*)vpSrc, usCnt, 0, (struct sockaddr*)&client_addr, &client_addr_length);
	//return recvfrom(server_socket_fd, (char*)vpSrc, usCnt, 0, (struct sockaddr*)&client_addr, &client_addr_length);
}

