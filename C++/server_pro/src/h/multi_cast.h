#pragma once

#include <WinSock2.h>

class CMultiCastServer
{
public:
	CMultiCastServer();
	~CMultiCastServer();

	void setLocalPort(int port);
	void setRemoteAddr(const char* ip,int port);

	void init();		// ��ʼ��
	void close();		// �ر�
	void setBroadCast();
	void setMultiCast();
	void start();
	int  sendUdp(const char* buf, size_t n, int opt, const struct sockaddr_in& addr);
private:
	SOCKET m_hSocket;				// socket ʵ��

	int    m_iLocalPort;			// ���ض˿ڣ�����ֻ��Ҫһ���˿ھͿ�����
	u_long m_ulRemoteIp;			// Զ��ip
	int	   m_iRemotePort;			// Զ�˶˿�

	bool   m_bIsSetBroad;
	bool   m_bIsSetMulti;
};