#pragma once
#ifndef LIBMONCLIENT
#define LIBMONCLIENT
#define _CRT_SECURE_NO_WARNINGS
#include "stdemace.h"
///*
//	InitServer called once by a server application.
//
//	Starts a listener thread that:
//	- Initializes network sockets
//	- provides network services to libMonClient
//
//	Implement the thread
//
//*/
//void InitServer(char* szListenIPAddr, unsigned short listenPort);



/*
	InitClient called once by client application.

	Peforms any necessary network socket intialization
*/
void InitClient(char* szConnectIPAddr, unsigned short connectPort);
void CloseClient();


//networking commands
constexpr char command_shutdown = 1 << 0;
constexpr char command_msg		= 1 << 1;
constexpr char command_msgcount = 1 << 2;
constexpr char command_errcount = 1 << 3;

//helper functions
char* PackHeader(char* dst, char command);
template<typename T>
char* PackData(char* dst, T* data);


void ShutdownMT(S16BIT devNum);
void GetMTMsg(S16BIT devNum, U16BIT* pCmdWrd1, U16BIT* pCmdWrd2, MSGSTRUCT* pMsg);
void GetMTMsgCount(S16BIT devNum, unsigned int* pMsgCount);
void GetMTMsgErrCount(S16BIT devNum, unsigned int* pMsgErrCount);

template<typename T>
inline char* PackData(char* dst, T* data)
{
	memcpy(dst, data, sizeof(T));
	return dst + sizeof(T);
}


#endif // !LIBMONCLIENT

