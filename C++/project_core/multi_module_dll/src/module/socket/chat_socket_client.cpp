#include "pch.h"
#include "base_socket.h"
#include "chat_socket_client.h"
#include "jsoncpp/include/json/json.h"
#include <iostream>


IChatSocketClient* DT_CreateChatSocketClient()
{
	IChatSocketClient* socketClient = new CChatSocketClient();
	return socketClient;
}


CChatSocketClient::CChatSocketClient()
	: m_iLocalPort(0)
	, m_hSocketCli(INVALID_SOCKET)
	, m_bIsConnected(false)
{
}

CChatSocketClient::~CChatSocketClient()
{

}

void CChatSocketClient::setLocalProt(int port)
{
	srand((unsigned int)time(NULL));
	m_iLocalPort = port + rand() % 101;
}

void CChatSocketClient::init()
{
	if (m_hSocketCli) close();

	m_hSocketCli = socket(AF_INET, SOCK_STREAM, 0);
	if (m_hSocketCli == INVALID_SOCKET)
	{
		wprintf(L"socket function failed with error: %u\n", WSAGetLastError());

		return;
	}

	m_addrCli.sin_addr.s_addr = INADDR_ANY;
	m_addrCli.sin_family = AF_INET;
	printf("��ǰ�˿ں� =====  %d\n", m_iLocalPort);
	m_addrCli.sin_port = htons(m_iLocalPort);

	m_addrSer.sin_addr.s_addr = inet_addr(CHAT_SERVER_SOCKET_IP);
	m_addrSer.sin_family = AF_INET;
	m_addrSer.sin_port = htons(CHAT_SERVER_SOCKET_PROT);

	int ret = bind(m_hSocketCli, (sockaddr*)&m_addrCli, sizeof(m_addrCli));
	if (ret == SOCKET_ERROR)
	{
		wprintf(L"bind failed with error %u\n", WSAGetLastError());
		close();
		return;
	}

	// socket Ĭ��������ʽ�ġ� accept �� recv ��������ʽ�ġ�
	u_long on = 1;
	ioctlsocket(m_hSocketCli, FIONBIO, &on);
}

void CChatSocketClient::close()
{
	closesocket(m_hSocketCli);
	m_hSocketCli = INVALID_SOCKET;
}

void CChatSocketClient::start()
{
	if (startRunning())
		printf("==================welcome. start================\n");
	else return;
	while (true)
	{
		char recvBuf[SOCKET_BUF_SIZE];
		memset(recvBuf, 0, sizeof(recvBuf));
		int ret = connect(m_hSocketCli, (sockaddr*)&m_addrSer, sizeof(m_addrSer));

		if (ret == SOCKET_ERROR)
		{
			int r = WSAGetLastError();
			if (r == WSAEWOULDBLOCK || r == WSAEINVAL)
			{
				Sleep(20);
				continue;
			}
			else if (r == WSAEISCONN)//�׽���ԭ���Ѿ����ӣ���
			{
				break;
			}
			else
			{
				std::cout << "��������" << std::endl;
				return;
			}
		}

		if (ret != SOCKET_ERROR)
		{
			printf("���ӳɹ�\n");
			break;
			//ret = recv(m_hSocketCli, recvBuf, sizeof(recvBuf), 0);
			//if (ret > 0) onReceiveMsg(recvBuf);
		}

	}
	m_bIsConnected = true;
}

bool CChatSocketClient::isConnected()
{
	return m_bIsConnected;
}

bool CChatSocketClient::disconnect()
{
	m_bIsConnected = false;
	return true;
}

void CChatSocketClient::onReceiveMsg(const string& msg)
{
	printf("������Ϣ��msg = %s\n", msg.data());
	Json::Reader reader(Json::Features::strictMode());
	Json::Value root;
	// reader��Json�ַ���������root��root������Json��������Ԫ��
	if (reader.parse(msg, root))
	{
		if (!root["sortid"].isNull())
		{
			std::string sortid = root["sortid"].asString();
			if (sortid == "connect")
			{
				if (!root["result"].isNull())
				{
					std::string method = root["result"].asString();
					if (method == "sucess")
					{
						printf("receive msg = sucess\n");
					}
				}
			}

		}
	}
}

void CChatSocketClient::onSendMsg(const string& msg)
{
	int ret = 0;
	ret = send(m_hSocketCli, msg.data(), sizeof(msg.data()), 0);
	if (ret > 0)
	{
		printf("client send msg = sucess\n");
	}
}

bool CChatSocketClient::calc()
{
	time_t t;
	struct tm* local;
	char T[256];
	memset(T, 0, 256);
	t = time(NULL);
	local = localtime(&t);
	sprintf(T, "%d/%d/%d %d:%d:%d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	strcpy(m_pSendData, T);
	return true;
}

bool CChatSocketClient::startRunning()
{
	m_hRecvThread = CreateThread(NULL, 0, recvThread, (void*)this, 0, NULL);//����static��Ա�������޷��������Ա����˴���thisָ�롣
	if (m_hRecvThread == NULL)
	{
		return false;
	}
	m_hSendThread = CreateThread(NULL, 0, sendThread, (void*)this, 0, NULL);
	if (m_hSendThread == NULL)
	{
		return false;
	}
	return true;
}

void CChatSocketClient::startChat()
{
	while (true)
	{
		cin >> m_pSendData;

		m_bSendPermission = true;

	}
}

SOCKET CChatSocketClient::getSocketClient()
{
	return m_hSocketCli;
}

DWORD __stdcall CChatSocketClient::sendThread(void* param)
{
	std::cout << "���������߳̿�ʼ���У���" << std::endl;
	CChatSocketClient* pClient = static_cast<CChatSocketClient*>(param);//���CClient����ָ�롣�Ա���ݳ�Ա������
	WaitForSingleObject(pClient->m_hEvent, INFINITE);//�ȴ����������߳�֪ͨ��
	while (pClient->isConnected())
	{
		while (pClient->getSendPermission())//���Է������ݡ�
		{
			std::cout << "�ȴ����������߳�֪ͨ����" << std::endl;
			//
			//ResetEvent(pClient->m_hEvent);
			int ret = send(pClient->getSocketClient(), pClient->m_pSendData, 1024, 0);
			if (ret == SOCKET_ERROR)
			{
				int r = WSAGetLastError();
				if (r == WSAEWOULDBLOCK)
				{
					continue;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				std::cout << "������ͳɹ�����" << std::endl;
				pClient->setSendPermission(false);
				break;
			}

		}
		Sleep(1000);//δ�յ�����֪ͨ��˯��1�롣

	}
}

DWORD __stdcall CChatSocketClient::recvThread(void* param)
{
	std::cout << "���������߳̿�ʼ���У���" << std::endl;
	CChatSocketClient* pClient = static_cast<CChatSocketClient*>(param);
	while (pClient->isConnected())
	{
		memset(pClient->m_pRecvData, 0, 1024);
		int ret = recv(pClient->getSocketClient(), pClient->m_pRecvData, 1024, 0);
		if (ret == SOCKET_ERROR)
		{
			int r = WSAGetLastError();
			if (r == WSAEWOULDBLOCK)
			{
				//std::cout<<"û���յ����Կͻ��˵����ݣ���"<<std::endl;
				Sleep(20);
				continue;
			}
			else if (r == WSAENETDOWN)
			{
				std::cout << "���������̳߳��ִ���,�����жϣ�" << std::endl;
				break;
			}
			else
			{
				std::cout << "���������̳߳��ִ���" << std::endl;
				break;
			}
		}
		else
		{
			std::cout << "��ϲ���յ����Կͻ��˵�����:" << pClient->m_pRecvData << std::endl;
			pClient->calc();
			std::cout << "֪ͨ�����̷߳��ͽ������" << std::endl;
			SetEvent(pClient->m_hEvent);
			pClient->setSendPermission(true);
		}
	}
	return 0;
}

void CChatSocketClient::setSendPermission(bool isSend)
{
	m_bSendPermission = isSend;
}

bool CChatSocketClient::getSendPermission()
{
	return m_bSendPermission;
}
