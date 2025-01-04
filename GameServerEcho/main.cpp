
#include <WinSock2.h>
#include "GameEcho.h"
GameEcho g_Echo;
int main()
{
	g_Echo.Start();
	g_Echo.WaitUntilShutDown();
	return 0;
}