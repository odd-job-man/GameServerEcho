#include <WinSock2.h>
#include <time.h>
#include "GameEcho.h"
#include "Player.h"
#include "Timer.h"
#include "ContentsType.h"
#include "EchoSingleThread.h"
#include "AuthMultiThread.h"
#include "Parser.h"


GameEcho::GameEcho()
    :GameServer{L"GameServerConfig.txt"}
{
}

void GameEcho::Start()
{
    SetEntirePlayerMemory(sizeof(Player));

    MonitoringUpdate* pMonitorThread = new MonitoringUpdate{ hcp_,1000,3 };
    Scheduler::Register_UPDATE(pMonitorThread);

	pAuthMulti_ = new AuthMultiThread{ this };
	ContentsBase::RegisterContents(en_ContentsType::AUTH, pAuthMulti_);
    ContentsBase::SetContentsToFirst(en_ContentsType::AUTH);

	char* pStart;
	PARSER psr = CreateParser(L"GameServerConfig.txt");
	GetValue(psr, L"TICK_PER_FRAME", (PVOID*)&pStart, nullptr);
	DWORD tick = (DWORD)_wtoi((LPCWSTR)pStart);

    pEchoSingle_ = new EchoSingleThread{ tick,hcp_,10,this };
	ContentsBase::RegisterContents(en_ContentsType::ECHO, pEchoSingle_);
	Scheduler::Register_UPDATE(pEchoSingle_);
    ReleaseParser(psr);

    pLanClient_ = new CMClient{ L"GameLanClientConfig.txt",SERVERNUM::GAME };
    pLanClient_->Start();

    pMonitorThread->RegisterMonitor(this);

    Scheduler::Start();
    InitialAccept();

    for (DWORD i = 0; i < IOCP_WORKER_THREAD_NUM_; ++i)
        ResumeThread(hIOCPWorkerThreadArr_[i]);
}

BOOL GameEcho::OnConnectionRequest(const SOCKADDR_IN* pSockAddrIn)
{
    return TRUE;
}


void* GameEcho::OnAccept(void* pPlayer)
{
    ContentsBase::FirstEnter(pPlayer);
    return nullptr;
}

void GameEcho::OnError(ULONGLONG id, int errorType, Packet* pRcvdPacket)
{
}

void GameEcho::OnPost(void* order)
{
}

void GameEcho::OnLastTaskBeforeAllWorkerThreadEndBeforeShutDown()
{
}

void GameEcho::OnResourceCleanAtShutDown()
{
}

void GameEcho::OnMonitor()
{
    FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUsertTime;
    FILETIME ftCurTime;
    GetProcessTimes(GetCurrentProcess(), &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUsertTime);
    GetSystemTimeAsFileTime(&ftCurTime);

    ULARGE_INTEGER start, now;
    start.LowPart = ftCreationTime.dwLowDateTime;
    start.HighPart = ftCreationTime.dwHighDateTime;
    now.LowPart = ftCurTime.dwLowDateTime;
    now.HighPart = ftCurTime.dwHighDateTime;

    ULONGLONG ullElapsedSecond = (now.QuadPart - start.QuadPart) / 10000 / 1000;

    ULONGLONG temp = ullElapsedSecond;

    ULONGLONG ullElapsedMin = ullElapsedSecond / 60;
    ullElapsedSecond %= 60;

    ULONGLONG ullElapsedHour = ullElapsedMin / 60;
    ullElapsedMin %= 60;

    ULONGLONG ullElapsedDay = ullElapsedHour / 24;
    ullElapsedHour %= 24;

    monitor.UpdateCpuTime(nullptr, nullptr);
    monitor.UpdateQueryData();

    ULONGLONG acceptTPS = InterlockedExchange(&acceptCounter_, 0);
    ULONGLONG disconnectTPS = InterlockedExchange(&disconnectTPS_, 0);
    ULONGLONG recvTPS = InterlockedExchange(&recvTPS_, 0);
    LONG sendTPS = InterlockedExchange(&sendTPS_, 0);
    LONG sessionNum = InterlockedXor(&lSessionNum_, 0);
    LONG authPlayerNum = InterlockedXor(&authPlayerNum_, 0);
    LONG echoFPS = InterlockedExchange(&pEchoSingle_->fps_, 0);
    acceptTotal_ += acceptTPS;

    double processPrivateMByte = monitor.GetPPB() / (1024 * 1024);
    double nonPagedPoolMByte = monitor.GetNPB() / (1024 * 1024);
    double availableByte = monitor.GetAB();
    double networkSentBytes = monitor.GetNetWorkSendBytes();
    double networkRecvBytes = monitor.GetNetWorkRecvBytes();

    static int shutDownFlag = 10;
    static int sdfCleanFlag = 0; // 1분넘어가면 초기화

    printf(
        "Elapsed Time : %02lluD-%02lluH-%02lluMin-%02lluSec\n"
        "Remaining HOME Key Push To Shut Down : %d\n"
        "Packet Pool Alloc Capacity : %d\n"
        "Packet Pool Alloc UseSize: %d\n"
        "SendQ Pool Capacity : %d\n"
        "Accept TPS: %llu\n"
        "Accept Total : %llu\n"
        "Disconnect TPS: %llu\n"
        "Recv Msg TPS: %llu\n"
        "Send Msg TPS: %d\n"
        "Session Num : %d\n"
        "AuthPlayer Num : %d\n"
        "EchoPlayer Num : %d\n"
        "Echo Thread FPS : %d\n"
        "----------------------\n"
        "Process Private MBytes : %.2lf\n"
        "Process NP Pool KBytes : %.2lf\n"
        "Memory Available MBytes : %.2lf\n"
        "Machine NP Pool MBytes : %.2lf\n"
        "Processor CPU Time : %.2f\n"
        "Process CPU Time : %.2f\n"
        "TCP Retransmitted/sec : %.2f\n\n",
        ullElapsedDay, ullElapsedHour, ullElapsedMin, ullElapsedSecond,
        shutDownFlag,
        Packet::packetPool_.capacity_ * Bucket<Packet, false>::size,
        Packet::packetPool_.AllocSize_,
        pSessionArr_[0].sendPacketQ_.nodePool_.capacity_,
        acceptTPS,
        acceptTotal_,
        disconnectTPS,
        recvTPS,
        sendTPS,
        sessionNum,
        authPlayerNum,
        echoPlayerNum_,
        echoFPS,
        processPrivateMByte,
        monitor.GetPNPB() / 1024,
        availableByte,
        nonPagedPoolMByte,
        monitor._fProcessorTotal,
        monitor._fProcessTotal,
        monitor.GetRetranse()
    );

    ++sdfCleanFlag;
    if (sdfCleanFlag == 60)
    {
        shutDownFlag = 10;
        sdfCleanFlag = 0;
    }

    if (GetAsyncKeyState(VK_HOME) & 0x0001)
    {
        --shutDownFlag;
        if (shutDownFlag == 0)
        {
            printf("Start ShutDown !\n");
            RequestShutDown();
            return;
        }
    }

    if (pLanClient_->bLogin_ == FALSE)
        return;

    time_t curTime;
    time(&curTime);

#pragma warning(disable : 4244) // 프로토콜이 4바이트를 받고 상위4바이트는 버려서 별수없음
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_SERVER_RUN, (int)1, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_SERVER_CPU, (int)monitor._fProcessTotal, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_SERVER_MEM, (int)processPrivateMByte, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_SESSION, (int)sessionNum, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER, (int)authPlayerNum, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER, (int)echoPlayerNum_, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS, (int)acceptTPS, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_PACKET_RECV_TPS, (int)recvTPS, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS, (int)sendTPS, curTime);
    pLanClient_->SendToMonitoringServer(GAME, dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS, (int)echoFPS, curTime);
    pLanClient_->SendToMonitoringServer(CHAT, dfMONITOR_DATA_TYPE_GAME_PACKET_POOL, (int)Packet::packetPool_.AllocSize_, curTime);

    pLanClient_->SendToMonitoringServer(CHAT, dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL, (int)monitor._fProcessorTotal, curTime);
    pLanClient_->SendToMonitoringServer(CHAT, dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY, (int)nonPagedPoolMByte, curTime);
    pLanClient_->SendToMonitoringServer(CHAT, dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV, (int)(networkRecvBytes / (float)1024), curTime);
    pLanClient_->SendToMonitoringServer(CHAT, dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND, (int)(networkSentBytes / (float)1024), curTime);
    pLanClient_->SendToMonitoringServer(CHAT, dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY, (int)availableByte, curTime);
#pragma warning(default : 4244)
}

