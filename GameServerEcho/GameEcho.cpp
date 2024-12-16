#include <WinSock2.h>
#include "GameEcho.h"
#include "Player.h"
#include "Timer.h"
#include "ContentsType.h"
#include "AuthSingleThread.h"
#include "EchoSingleThread.h"
#include "AuthMultiThread.h"
#include "EchoMultiThread.h"
#include "Parser.h"


void GameEcho::Start()
{
    SetEntirePlayerMemory(maxPlayer_, sizeof(Player));

	char* pStart;
	PARSER psr = CreateParser(L"ServerConfig.txt");
    GetValue(psr, L"AUTH_SINGLE", (PVOID*)&pStart, nullptr);
    int bAuthSingle = (int)_wtoi((LPCWSTR)pStart);

    GetValue(psr, L"ECHO_SINGLE", (PVOID*)&pStart, nullptr);
    int bEchoSingle = (int)_wtoi((LPCWSTR)pStart);
	ReleaseParser(psr);

    MonitoringUpdate* pMonitorThread = new MonitoringUpdate{ hcp_,1000,3 };
    Timer::Reigster_UPDATE(pMonitorThread);

    if (bAuthSingle == 1)
    {
        auto* pAuthThread = new AuthSingleThread{ 20,hcp_,10,this };
        pMonitorThread->RegisterMonitor(pAuthThread);
        ContentsBase::RegisterContents(en_ContentsType::AUTH, pAuthThread);
        Timer::Reigster_UPDATE(pAuthThread);
    }
    else
    {
        auto* pAuthThread = new AuthMultiThread{ this };
        pMonitorThread->RegisterMonitor(pAuthThread);
        ContentsBase::RegisterContents(en_ContentsType::AUTH, pAuthThread);
    }
    ContentsBase::SetContentsToFirst(en_ContentsType::AUTH);

    if (bEchoSingle == 1)
    {
        auto* pEchoThread = new EchoSingleThread{ 20,hcp_,10,this };
        pMonitorThread->RegisterMonitor(pEchoThread);
        ContentsBase::RegisterContents(en_ContentsType::ECHO, pEchoThread);
        Timer::Reigster_UPDATE(pEchoThread);
    }
    else
    {
        auto* pEchoThread = new EchoMultiThread{ this };
        pMonitorThread->RegisterMonitor(pEchoThread);
        ContentsBase::RegisterContents(en_ContentsType::ECHO, pEchoThread);
    }

    Timer::Start();

    for (DWORD i = 0; i < IOCP_WORKER_THREAD_NUM_; ++i)
        ResumeThread(hIOCPWorkerThreadArr_[i]);

    ResumeThread(hAcceptThread_);
}

BOOL GameEcho::OnConnectionRequest()
{
    if (lSessionNum_ + 1 > maxSession_)
        return FALSE;
    else
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
    //((Excutable*)order)->Excute();
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
    ULONGLONG acceptTPS = InterlockedExchange(&acceptCounter_, 0);
    ULONGLONG disconnectTPS = InterlockedExchange(&disconnectTPS_, 0);
    ULONGLONG recvTPS = InterlockedExchange(&recvTPS_, 0);
    LONG sendTPS = InterlockedExchange(&sendTPS_, 0);
    acceptTotal_ += acceptTPS;

}

