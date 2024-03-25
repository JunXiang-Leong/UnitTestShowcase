#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_BUFLEN 512
#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include "libMonClient.h"

static SOCKET ConnectSocket = INVALID_SOCKET;

void InitClient(char* szConnectIPAddr, unsigned short connectPort)
{
	WSADATA wsaData;
	
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	const char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return ;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char port[5];
	sprintf(port, "%u", connectPort);
	// Resolve the server address and port
	iResult = getaddrinfo(szConnectIPAddr, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// Receive until the peer closes the connection
	//do {

	//	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	//	if (iResult > 0)
	//		printf("Bytes received: %d\n", iResult);
	//	else if (iResult == 0)
	//		printf("Connection closed\n");
	//	else
	//		printf("recv failed with error: %d\n", WSAGetLastError());

	//} while (iResult > 0);


}
void CloseClient()
{
	closesocket(ConnectSocket);
	WSACleanup();

	return;
}

char* PackHeader(char* dst, char command)
{
	memcpy(dst, &command, sizeof(char));
	return dst + 1;
}

void ShutdownMT(S16BIT devNum)
{
	char arr[DEFAULT_BUFLEN];
	char* next_data = PackHeader(arr, command_shutdown);
	next_data = PackData(next_data, &devNum);
	*next_data = '\0';//end the msg
	send(ConnectSocket, arr, (next_data - arr) + 1, 0);
	int iResult = recv(ConnectSocket, arr, sizeof(arr), 0);
	if (iResult > 0)
	{
		printf("Bytes received: %d\n", iResult);
		printf(arr);
	}
	else
	{
		printf("Failed to receive anything\n");
	}
}
void GetMTMsg(S16BIT devNum, U16BIT* pCmdWrd1, U16BIT* pCmdWrd2, MSGSTRUCT* pMsg)
{
	char arr[DEFAULT_BUFLEN];

	char* next_data = PackHeader(arr, command_msg);
	next_data = PackData(next_data, &devNum);
	next_data = PackData(next_data, pCmdWrd1);
	next_data = PackData(next_data, pCmdWrd2);
	*next_data = '\0';//end the msg
	//send our custom package
	send(ConnectSocket, arr, (next_data - arr)  + 1, 0);

	//blocking call //wait for response
	int iResult = recv(ConnectSocket, arr, sizeof(arr), 0);
	if (iResult > 0)
	{
		printf("Bytes received: %d\n", iResult);
		memcpy(pMsg, arr, sizeof(MSGSTRUCT));
	}
	else
	{
		printf("Failed to receive anything\n");
	}
}
void GetMTMsgCount(S16BIT devNum, unsigned int* pMsgCount)
{
	char arr[DEFAULT_BUFLEN];
	char* next_data = PackHeader(arr, command_msgcount);
	next_data = PackData(next_data, &devNum);
	*next_data = '\0';//end the msg
	send(ConnectSocket, arr, (next_data - arr) + 1, 0);
	int iResult = recv(ConnectSocket, arr, sizeof(arr), 0);
	if (iResult > 0)
	{
		printf("Bytes received: %d\n", iResult);
		memcpy(pMsgCount, arr, sizeof(unsigned int));
	}
	else
	{
		printf("Failed to receive anything\n");
	}
}
void GetMTMsgErrCount(S16BIT devNum, unsigned int* pErrMsgCount)
{
	char arr[DEFAULT_BUFLEN];
	char* next_data = PackHeader(arr, command_errcount);
	next_data = PackData(next_data, &devNum);
	*next_data = '\0';//end the msg
	send(ConnectSocket, arr, (next_data - arr) + 1, 0);
	int iResult = recv(ConnectSocket, arr, sizeof(arr), 0);
	if (iResult > 0)
	{
		printf("Bytes received: %d\n", iResult);
		memcpy(pErrMsgCount, arr, sizeof(unsigned int));
	}
	else
	{
		printf("Failed to receive anything\n");
	}
}
