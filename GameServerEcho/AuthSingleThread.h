#pragma once
#include "ContentsBase.h"
#include "SerialContent.h"

class AuthSingleThread : public SerialContent
{
public:
	AuthSingleThread(DWORD tickPerFrame, HANDLE hCompletionPort, LONG pqcsLimit, GameServer* pGameServer);
	virtual void OnEnter(void* pPlayer) override;
	virtual void OnLeave(void* pPlayer) override;
	virtual void OnRecv(Packet* pPacket, void* pPlayer) override;
	virtual void ProcessEachPlayer() override;
};
