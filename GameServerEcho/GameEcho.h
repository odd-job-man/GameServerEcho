#pragma once
#include <Windows.h>
#include "GameServer.h"
#include "HMonitor.h"
#include "EchoSingleThread.h"
#include "EchoMultiThread.h"
#include "AuthSingleThread.h"
#include "AuthMultiThread.h"

class GameEcho : public GameServer
{
public:
	GameEcho();
	void Start();
	virtual BOOL OnConnectionRequest(const SOCKADDR_IN* pSockAddrIn) override; 
	virtual void* OnAccept(void* pPlayer) override;
	virtual void OnError(ULONGLONG id, int errorType, Packet* pRcvdPacket) override;
	virtual void OnPost(void* order) override;

	EchoSingleThread* pEchoSingle_ = nullptr;
	EchoMultiThread* pEchoMulti_ = nullptr;
	AuthSingleThread* pAuthSingle_ = nullptr;
	AuthMultiThread* pAuthMulti_ = nullptr;

	int bAuthSingle_;
	int bEchoSingle_;

	// Monitorable Override
private:
	virtual void OnLastTaskBeforeAllWorkerThreadEndBeforeShutDown() override; 
	virtual void OnResourceCleanAtShutDown() override;
	virtual void OnMonitor() override;
	static inline HMonitor monitor;
};
