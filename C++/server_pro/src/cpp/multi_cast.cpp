#include "multi_cast.h"
#include <iostream>
#include <Ws2tcpip.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

const unsigned int RECVBUFSIZE = 1024 * 50;

CMultiCastServer::CMultiCastServer()
	: m_hSocket(INVALID_SOCKET)
	, m_iLocalPort(0)
	, m_ulRemoteIp(0)
	, m_iRemotePort(0)
	, m_bIsSetBroad(false)
	, m_bIsSetMulti(false)
{
}

CMultiCastServer::~CMultiCastServer()
{
}

void CMultiCastServer::setLocalPort(int port)
{
	m_iLocalPort = port;
}

void CMultiCastServer::setRemoteAddr(const char* ip, int port)
{
	// inet_addr ���������� IPv4 ���ʮ���Ƶ�ַ���ַ���ת��Ϊ IN_ADDR�ṹ����ȷ��ַ
	m_ulRemoteIp = inet_addr(ip);
	m_iRemotePort = port;
}

void CMultiCastServer::init()
{
	WSADATA wsaData;
	int iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"Error at WSAStartup()\n");
		return;
	}

	if (m_hSocket) close();
	// ����1:��ַ��淶,AF_INET  (IPv4) ��ַϵ��
	// ����2:SOCK_DGRAM ʹ�� Internet ��ַϵ�У�AF_INET �� AF_INET6�����û����ݱ�Э�� (UDP)
	//		 SOCK_STREAM ʹ�� Internet ��ַϵ�У�AF_INET �� AF_INET6���Ĵ������Э�� (TCP)
	// ����3:���ָ��ֵΪ 0��������߲�ϣ��ָ��Э�飬�����ṩ�߽�ѡ��Ҫʹ�õ�Э��
	m_hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_hSocket == INVALID_SOCKET)
	{
		wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return;
	}


	// sockaddr ���ݾ��������ֽ�˳���ʾ
	sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	// htons ������ �� ���� ת��Ϊ TCP/IP�����ֽ�˳�򣨴���ֽ��򣩷��� u_short
	localAddr.sin_port = htons(m_iLocalPort);
	// sin_addr.s_addr ��ʽΪu_long�� IPv4 ��ַ
	// htonl ������ �� ���� ת��Ϊ TCP/IP�����ֽ�˳�򣨴���ֽ��򣩷��� u_long
	localAddr.sin_addr.s_addr = INADDR_ANY;
	// bind ���������ص�ַ���׽��������
	int ret = bind(m_hSocket, (sockaddr*)&localAddr, sizeof(localAddr));
	if (ret == SOCKET_ERROR)
	{
		wprintf(L"bind failed with error %u\n", WSAGetLastError());
		close();
		WSACleanup();
		return;
	}
	else
		wprintf(L"bind returned success\n");

	u_long on = 1;
	// ioctlsocket ���������׽��ֵ� I/O ģʽ
	iResult = ioctlsocket(m_hSocket, FIONBIO, &on);
	if (iResult != NO_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", iResult);
	}

	in_addr inaddr;
	int remote_ip = inet_addr("233.0.0.1");
	inaddr.s_addr = remote_ip;

	unsigned char* val = (unsigned char*)&(remote_ip);
	//if (val[0] >= 224 && val[0] <= 239)
	//{
		int loop = 1;
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = remote_ip;
		mreq.imr_interface.s_addr = INADDR_ANY;
		ret = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*)&loop, sizeof(loop));
		if (ret != 0) return;

		ret = setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
		printf("CDgramMcast::join, IP_ADD_MEMBERSHIP: %s", inet_ntoa(inaddr));
	//}

}

void CMultiCastServer::close()
{
	closesocket(m_hSocket);
}

void CMultiCastServer::setBroadCast()
{
	in_addr inaddr;
	inaddr.s_addr = m_ulRemoteIp;
	unsigned char* ip = (unsigned char*)&m_ulRemoteIp;
	if ((ip[0] == 192 || ip[0] == 172 || ip[0] == 10 || ip[0] == 255) && ip[3] == 255)
	{
		int val = 1;
		// setsockopt���������׽���ѡ��
		int ret = setsockopt(m_hSocket, SOL_SOCKET, SO_BROADCAST,(const char*)&val,sizeof(val));
		if (ret == SOCKET_ERROR)
		{
			printf("set broad cast error : %d", WSAGetLastError());
		}
		// inet_ntoa ������ (Ipv4) Internet �����ַת��Ϊ Internet ��׼���ʮ���Ƹ�ʽ��ASCII �ַ���
		printf("setBroadCast ip: %s\n", inet_ntoa(inaddr));
	}
	m_bIsSetBroad = true;
}

void CMultiCastServer::setMultiCast()
{
}

void CMultiCastServer::start()
{
	char buf[RECVBUFSIZE];
	while (true)
	{
		memset(buf, 0, sizeof(buf));
		printf("��������Ҫ���͵���Ϣ:\n");
		std::cin >> buf;

		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(m_iRemotePort);
		address.sin_addr.s_addr = m_ulRemoteIp;


		int ret = sendUdp(buf, sizeof(buf), 0, address);
		printf("��Ϣ���ͽ�� = %d\n", ret);
	}
}

int CMultiCastServer::sendUdp(const char* buf, size_t n, int opt, const sockaddr_in& addr)
{
	int ret = sendto(m_hSocket, buf, n, opt, (struct sockaddr*)&addr, sizeof(addr));
	return ret;
}
