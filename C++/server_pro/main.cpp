#include "multi_cast.h"
#include "../multi_module_dll/src/include/base_socket.h"

int main()
{
	// �㲥�鲥demo
	//CMultiCastServer server;
	//server.setLocalPort(8888);
	//server.setRemoteAddr("192.168.1.255", 9999);
	//server.init();
	//server.setBroadCast();
	////server.setMultiCast();
	//server.start();

	// ������demo
	if (!DT_Start_WinSocket_Dll())
	{
		printf("��ʼ��ʧ��.");
		return 0;
	}

	IChatSocketServer* chatServer = DT_CreateChatSocketServer();
	if (chatServer)
	{
		chatServer->setLocalProt(12248);
		chatServer->init();
		chatServer->start();
	}


	DT_Clean_WinSocket_Dll();
	return 0;
}