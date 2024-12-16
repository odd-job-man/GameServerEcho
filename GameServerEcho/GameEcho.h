#pragma once
#include <Windows.h>
#include "GameServer.h"
#include "HMonitor.h"

class GameEcho : public GameServer
{
public:
	void Start();
	virtual BOOL OnConnectionRequest() override;
	virtual void* OnAccept(void* pPlayer) override;
	virtual void OnError(ULONGLONG id, int errorType, Packet* pRcvdPacket) override;
	virtual void OnPost(void* order) override;

	// Monitorable Override
	virtual void OnMonitor() override;
	static inline HMonitor monitor;
};
