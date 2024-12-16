#pragma once
#include "SerialContent.h"
#include "ContentsBase.h"

class EchoSingleThread : public SerialContent, public Monitorable
{
public:
	EchoSingleThread(DWORD tickPerFrame, HANDLE hCompletionPort, LONG pqcsLimit, GameServer* pGameServer);
	virtual void OnEnter(void* pPlayer) override;
	virtual void OnLeave(void* pPlayer) override;
	virtual void OnRecv(Packet* pPacket, void* pPlayer) override;
	virtual void ProcessEachPlayer() override;
	virtual void OnMonitor() override;
};
