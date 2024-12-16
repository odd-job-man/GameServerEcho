#pragma once
#include "ParallelContent.h"
class EchoMultiThread : public ParallelContent, public Monitorable
{
public:
	EchoMultiThread(GameServer* pGameServer);

	// ContentsBase overridng 
	virtual void OnEnter(void* pPlayer) override;
	virtual void OnLeave(void* pPlayer) override;
	virtual void OnRecv(Packet* pPacket, void* pPlayer) override;

	// Monitorable Overriding
	virtual void OnMonitor() override;
};
