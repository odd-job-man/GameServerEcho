#include <WinSock2.h>
#include "CommonProtocol.h"
#include "GameEcho.h"
#include "EchoSingleThread.h"
#include "Player.h"

void MAKE_SC_PACKET_CS_GAME_RES_LOGIN(BYTE status, INT64 accountNo, SmartPacket& sp)
{
	(*sp) << (WORD)en_PACKET_CS_GAME_RES_LOGIN << status << accountNo;
}

EchoSingleThread::EchoSingleThread(DWORD tickPerFrame, HANDLE hCompletionPort, LONG pqcsLimit, GameServer* pGameServer)
	:SerialContent{ tickPerFrame,hCompletionPort,pqcsLimit,pGameServer }
{
}

void EchoSingleThread::OnEnter(void* pPlayer)
{
	Player* pEnterPlayer = (Player*)pPlayer;
	SmartPacket sp = PACKET_ALLOC(Net);
	MAKE_SC_PACKET_CS_GAME_RES_LOGIN(1, pEnterPlayer->accountNo, sp);
	pGameServer_->SendPacket(pEnterPlayer->sessionID,sp);
	++static_cast<GameEcho*>(pGameServer_)->echoPlayerNum_;
}

void EchoSingleThread::OnLeave(void* pPlayer)
{
	--static_cast<GameEcho*>(pGameServer_)->echoPlayerNum_;
}

void EchoSingleThread::OnRecv(Packet* pPacket, void* pPlayer)
{
	WORD Type;
	(*pPacket) >> Type;
	switch ((en_PACKET_TYPE)Type)
	{
	case en_PACKET_CS_GAME_REQ_ECHO:
	{
		INT64 accountNo;
		LONGLONG sendTick;
		(*pPacket) >> accountNo >> sendTick;
		SmartPacket sp = PACKET_ALLOC(Net);
		(*sp) << (WORD)en_PACKET_CS_GAME_RES_ECHO << accountNo << sendTick;
		pGameServer_->EnqPacket(((Player*)pPlayer)->sessionID, sp.GetPacket());
		break;
	}
	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
	{
		SmartPacket sp = PACKET_ALLOC(Net);
		(*sp) << Type;
		pGameServer_->EnqPacket(((Player*)pPlayer)->sessionID, sp.GetPacket());
		break;
	}
	default:
		pGameServer_->Disconnect(((Player*)pPlayer)->sessionID);
		__debugbreak();
		break;
	}
}

void EchoSingleThread::ProcessEachPlayer()
{
}

