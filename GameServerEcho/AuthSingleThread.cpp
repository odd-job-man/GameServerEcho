#include <WinSock2.h>
#include "GameServer.h"
#include "Player.h"
#include "ContentsType.h"
#include "AuthSingleThread.h"
#include "CommonProtocol.h"
#include "Packet.h"


AuthSingleThread::AuthSingleThread(DWORD tickPerFrame, HANDLE hCompletionPort, LONG pqcsLimit, GameServer* pGameServer)
	:SerialContent{ tickPerFrame,hCompletionPort,pqcsLimit,pGameServer }
{}

void AuthSingleThread::OnEnter(void* pPlayer)
{
}

void AuthSingleThread::OnLeave(void* pPlayer)
{
	InterlockedDecrement(&pGameServer_->lPlayerNum_);
}

void AuthSingleThread::OnRecv(Packet* pPacket, void* pPlayer)
{
	Player* pAuthPlayer = (Player*)pPlayer;
	WORD Type;
	(*pPacket) >> Type;

	switch ((en_PACKET_TYPE)Type)
	{
	case en_PACKET_CS_GAME_REQ_LOGIN:
	{
		INT64 accountNo;
		int version;
		(*pPacket) >> accountNo;
		pPacket->MoveReadPos(64);
		(*pPacket) >> version;

		pAuthPlayer->accountNo = accountNo;
		pAuthPlayer->sessionID = pGameServer_->GetSessionID(pPlayer);
		RegisterLeave(pAuthPlayer, en_ContentsType::ECHO);
		break;
	}
	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
	{
		SmartPacket sp = PACKET_ALLOC(Net);
		(*sp) << Type;
		pGameServer_->SendPacket(pAuthPlayer->sessionID, sp);
		break;
	}
	default:
		pGameServer_->Disconnect(pAuthPlayer->sessionID);
		__debugbreak();
		break;
	}
}

void AuthSingleThread::ProcessEachPlayer()
{
}

