#pragma once
#include "ParallelContent.h"

class AuthMultiThread : public ParallelContent, public Monitorable
{
public:
	AuthMultiThread(GameServer* pGameServer);

	// ContentsBase overridng 
	virtual void OnEnter(void* pPlayer) override;
	virtual void OnLeave(void* pPlayer) override;
	virtual void OnRecv(Packet* pPacket, void* pPlayer) override;

	// Monitorable Overriding
	virtual void OnMonitor() override;
};