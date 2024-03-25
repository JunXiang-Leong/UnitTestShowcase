#pragma comment(lib,"ws2_32")
#define _WINSOCKAPI_
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <atomic>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include "libMon.h"


struct MT_MessageLog{
	MSGSTRUCT msgContainer[100];
	std::atomic_uint messageCount = 0;
	std::atomic_uint errMessageCount = 0;
	char buffer[512];
	bool endPoll = false;
};
static SOCKET ListenSocket = INVALID_SOCKET;
static SOCKET ClientSocket = INVALID_SOCKET;
 
static MT_MessageLog devices[32]; //since device numbers range from 0 - 31 this is enough
S16BIT InitMT(S16BIT devNum)
{
	S16BIT wResult = aceInitialize(devNum, ACE_ACCESS_CARD, ACE_MODE_MT, 0, 0, 0);
	if (wResult)
	{
		printf("aceInitalize Failed");
		//PrintOutError(wResult);
		return 1;
	}
	S16BIT nResult = 0;
	MSGSTRUCT sMsg;
	//since I dont know the size of the actual MSGSTRUCT or what size msgContainter should be it is better off i do this
	MT_MessageLog* msgLog = &devices[devNum];
	const unsigned int sizeMsgContainer = (sizeof(msgLog->msgContainer) / sizeof(MSGSTRUCT));
	while (msgLog->endPoll)
	{
		//get the next message and remove from the stack
		nResult = aceMTGetStkMsgDecoded(devNum, &sMsg, ACE_MT_MSGLOC_NEXT_PURGE, ACE_MT_STKLOC_ACTIVE);
		if (nResult == 1)
		{
			++msgLog->messageCount;
			if (sMsg.wBlkSts & 0x800) //to get the error flag at bit 12
			{
				++msgLog->errMessageCount;
			}
			
			unsigned int index = msgLog->messageCount % sizeMsgContainer;
			msgLog->msgContainer[index] = sMsg; //direct copy
		}
	}
	msgLog->endPoll = false;
	msgLog->errMessageCount = 0;
	msgLog->messageCount = 0;
	aceFree(devNum);
	return S16BIT();
}

S16BIT ShutdownMT(S16BIT devNum)
{
	if(devNum >= 0 && devNum < 32)
		devices[devNum].endPoll = true;//technically ok because this is the only source of write for this variable
	aceFree(devNum);
	return S16BIT(0);
}


S16BIT GetMTMsg(S16BIT devNum, U16BIT* pCmdWrd1, U16BIT* pCmdWrd2, MSGSTRUCT* pMsg)
{
	if (devNum < 0 || devNum > 31)
		return 1;

	MT_MessageLog* msgLog = &devices[devNum];
	const unsigned int sizeMsgContainer = (sizeof(msgLog->msgContainer) / sizeof(MSGSTRUCT));
	const unsigned int populatedMsgSize = msgLog->messageCount < sizeMsgContainer ? (unsigned int)msgLog->messageCount : sizeMsgContainer;
	for (unsigned int i = 0; i < populatedMsgSize; ++i)
	{
		MSGSTRUCT* myMsg = &msgLog->msgContainer[i];
		//if true then we found the target
		if (myMsg->wCmdWrd1 == *pCmdWrd1 && ((pCmdWrd2 == NULL) || (myMsg->wCmdWrd2 == *pCmdWrd1)))
		{
			*pMsg = *myMsg;
		}
	}
	return S16BIT(0);
}

S16BIT GetMTMsgCount(S16BIT devNum, unsigned int* pMsgCount)
{
	if (devNum >= 0 && devNum < 32)
		*pMsgCount = devices[devNum].messageCount;
	else
		return 1;
	return 0;
}

S16BIT GetMTMsgErrCount(S16BIT devNum, unsigned int* pMsgErrCount)
{
	if (devNum >= 0 && devNum < 32)
		*pMsgErrCount = devices[devNum].errMessageCount;
	else
		return 1;
	return S16BIT();
}
#define DEFAULT_BUFLEN 512
void InitServer(char* szListenIPAddr, unsigned short listenPort)
{
	WSADATA wsaData;
	int iResult;

	

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	// Resolve the local address and port to be used by the server
	char port[5];
	sprintf(port, "%u", listenPort);
	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	// Create a SOCKET for the server to listen for client connections.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			char header = recvbuf[0];
			char* next_data = recvbuf + 1;
			S16BIT devNum = 0;
			memcpy(&devNum, next_data, sizeof(S16BIT));
			next_data = next_data + sizeof(S16BIT);
				
			switch (recvbuf[0])
			{
			case 1: //shutdown
			{
				ShutdownMT(devNum);
				memset(recvbuf, 0, sizeof(recvbuf));//not needed but makes it easier to debug
				char message[] = "Device stated has been shutdown!/n/0";
				memcpy(recvbuf, message, sizeof(message));
				send(ClientSocket, recvbuf, sizeof(recvbuf), 0);
				break;
			}
			case 2: //get msg
			{
				U16BIT pCmdWrd1 = 0;
				U16BIT pCmdWrd2 = 0;
				memcpy(&pCmdWrd1, next_data, sizeof(U16BIT));
				next_data = next_data + sizeof(U16BIT);
				memcpy(&pCmdWrd2, next_data, sizeof(U16BIT));
				next_data = next_data + sizeof(U16BIT);

				MSGSTRUCT msg;
				GetMTMsg(devNum, &pCmdWrd1, &pCmdWrd2, &msg);

				memset(recvbuf, 0, sizeof(recvbuf));//not needed but makes it easier to debug
				memcpy(recvbuf, &msg, sizeof(MSGSTRUCT));
				send(ClientSocket, recvbuf, sizeof(recvbuf), 0);
				break;
			}
			case 4: //msg count
			{
				unsigned int count;
				GetMTMsgCount(devNum, &count);

				memset(recvbuf, 0, sizeof(recvbuf));//not needed but makes it easier to debug
				memcpy(recvbuf, &count, sizeof(unsigned int));
				send(ClientSocket, recvbuf, sizeof(recvbuf), 0);
				break;
			}
			case 8: //msg err count
			{
				unsigned int errCount;
				GetMTMsgCount(devNum, &errCount);

				memset(recvbuf, 0, sizeof(recvbuf));//not needed but makes it easier to debug
				memcpy(recvbuf, &errCount, sizeof(unsigned int));
				send(ClientSocket, recvbuf, sizeof(recvbuf), 0);
				break;
			}
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return;
}