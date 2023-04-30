#include "multi_cast.h"
#include <iostream>
#include <WS2tcpip.h>


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

	//// ���õ�ַ����ѡ��
	//int reuseEnable = 1;
	//if (setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseEnable, sizeof(reuseEnable)) < 0) {
	//	printf("setsockopt error");
	//	return;
	//}


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
	sockaddr_in from;
	socklen_t addr_len = sizeof(from);
	int len;
	while (true)
	{
		if (!m_hSocket) return;

		memset(buf, 0, sizeof(buf));
		len = recvfrom(m_hSocket, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&from, &addr_len);
		if (len > 0)
		{
			buf[len] = '\0';
			printf("RecvMessage From %s/%d:%s", inet_ntoa(from.sin_addr), ntohs(from.sin_port), buf);
		}
		Sleep(500);
	}
}

int CMultiCastServer::sendUdp(const char* buf, size_t n, int opt, const sockaddr_in& addr)
{
	int ret = sendto(m_hSocket, buf, n, opt, (struct sockaddr*)&addr, sizeof(addr));
	return 0;
}
