#pragma once
#ifndef LIBMON
#define LIBMON

#include "stdemace.h"


/*
typedef short S16BIT;
typedef unsigned short U16BIT;
typedef unsigned int U32BIT;
typedef struct empty{
	int count;
}MSGSTRUCT;
*/

/*
	InitMT called Once by application.

	Starts a thread that:
	- Initialize, Configure the device to Monitor (MT) Mode.
	- Polls and buffers 1553 messages from the hardware, using appropriate data
	  structures and synchronization objects that facilitate usage of GetMTXXX
	  functions from an application.

	Implement the thread.

	Hints:
	- Examine samples/emacepl/src folder
*/
S16BIT InitMT(S16BIT devNum);



/*
	ShutdownMT is called Once by application.

	Shuts down thread and MT hardware.
*/
S16BIT ShutdownMT(S16BIT devNum);



/*
	Called from application to retrieve buffered messages from the MT.

	Can be used to retrieve any message type specified in Section 1.2 of the
	MIL-STD-1553 Designer's Guide.

	1553 messages can either have one or two command words.
	When retrieving a single command word message, pCmdWrd2 will be set to NULL.

	Hints:
	- Examine MSGSTRUCT and Section 1.2, 1.3 of the MIL-STD-1553 Designer's
	  Guide
*/
S16BIT GetMTMsg(S16BIT devNum, U16BIT* pCmdWrd1, U16BIT* pCmdWrd2, MSGSTRUCT* pMsg);



/*
	Called from application to obtain total number of messages detected by
	Monitor since initialization.
*/
S16BIT GetMTMsgCount(S16BIT devNum, unsigned int* pMsgCount);

/*
	Called from application to obtain total number of erroneous messages
	detected by Monitor since initialization
*/
S16BIT GetMTMsgErrCount(S16BIT devNum, unsigned int* pMsgErrCount);

/*
	InitServer called once by a server application.

	Starts a listener thread that:
	- Initializes network sockets
	- provides network services to libMonClient

	Implement the thread

*/
void InitServer(char* szListenIPAddr, unsigned short listenPort);

#endif
