#include "libMon.h"

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#endif
#include <stdio.h>
#include <atomic>
struct MT_MessageLog{
	MSGSTRUCT msgContainer[100];
	std::atomic_uint messageCount = 0;
	std::atomic_uint errMessageCount = 0;
	bool endPoll = false;
};
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
	//aceFree(devNum);
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
