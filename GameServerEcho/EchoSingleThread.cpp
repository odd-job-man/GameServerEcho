#include <WinSock2.h>
#include "GameServer.h"
#include "CommonProtocol.h"
#include "EchoSingleThread.h"
#include "Player.h"

void MAKE_SC_PACKET_CS_GAME_RES_LOGIN(BYTE status, INT64 accountNo, SmartPacket& sp)
{
	(*sp) << (WORD)en_PACKET_CS_GAME_RES_LOGIN << status << accountNo;
}

void CS_MAKE_REQ_ECHO(INT64 accountNo, LONGLONG sendTick, SmartPacket& sp)
{
	(*sp) << (WORD)en_PACKET_CS_GAME_RES_ECHO << accountNo << sendTick;
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
	pGameServer_->SendPacket(pEnterPlayer->sessionID, sp);
}

void EchoSingleThread::OnLeave(void* pPlayer)
{
	InterlockedDecrement(&pGameServer_->lPlayerNum_);
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
		CS_MAKE_REQ_ECHO(accountNo, sendTick, sp);
		pGameServer_->SendPacket(pGameServer_->GetSessionID(pPlayer), sp);
		break;
	}
	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
	{
		SmartPacket sp = PACKET_ALLOC(Net);
		(*sp) << Type;
		pGameServer_->SendPacket(pGameServer_->GetSessionID(pPlayer), sp);
		break;
	}
	default:
		__debugbreak();
		break;
	}
}

void EchoSingleThread::ProcessEachPlayer()
{
}

