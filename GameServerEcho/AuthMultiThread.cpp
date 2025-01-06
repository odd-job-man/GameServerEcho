#include <WinSock2.h>
#include "CommonProtocol.h"
#include "Player.h"
#include "ContentsType.h"
#include "GameEcho.h"
#include "AuthMultiThread.h"
#include "Packet.h"

void MAKE_SC_PACKET_CS_GAME_RES_LOGIN(BYTE status, INT64 accountNo, SmartPacket& sp);

AuthMultiThread::AuthMultiThread(GameServer* pGameServer)
	:ParallelContent{pGameServer}
{
}

void AuthMultiThread::OnEnter(void* pPlayer)
{
	InterlockedIncrement(&static_cast<GameEcho*>(pGameServer_)->authPlayerNum_);
}

void AuthMultiThread::OnLeave(void* pPlayer)
{
	InterlockedDecrement(&static_cast<GameEcho*>(pGameServer_)->authPlayerNum_);
}

void AuthMultiThread::OnRecv(Packet* pPacket, void* pPlayer)
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
		pGameServer_->SendPacket(pGameServer_->GetSessionID(pPlayer), sp);
		break;
	}
	default:
		pGameServer_->Disconnect(pGameServer_->GetSessionID(pPlayer));
		__debugbreak();
		break;
	}
}
