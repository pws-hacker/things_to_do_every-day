#pragma once


//class CChatSocketServer;
class CChatSocketClient : public IChatSocketClient
{

public:
	CChatSocketClient();
	virtual ~CChatSocketClient();

	virtual void setLocalProt(int port);

	virtual void init();

	virtual void close();
	virtual void start();

	bool isConnected();
	bool disconnect();

	virtual void onReceiveMsg(const string& msg);
	virtual void onSendMsg(const string& msg);

	bool calc();//���㵱ǰʱ�䣬�����Ƶ����ͻ������ڡ�
	bool startRunning();//��ʼ���з��ͺͽ����̡߳�
	virtual void startChat();
	SOCKET getSocketClient();
	static DWORD WINAPI sendThread(void* param);//�����߳���ں�����
	static DWORD WINAPI recvThread(void* param);//�����߳���ں�����
	void setSendPermission(bool isSend);
	bool getSendPermission();
private:
	SOCKET m_hSocketCli;				// socket �ͻ���


	HANDLE m_hSendThread;//�����߳̾����
	HANDLE m_hRecvThread;//�����߳̾����

	char* m_pRecvData;//���ջ�������
	char* m_pSendData;//���ͻ�������
	HANDLE m_hEvent;//�����̺߳ͽ����߳�ͬ���¼����󡣽��տͻ��������֪ͨ�����̷߳��͵�ǰʱ�䡣

	sockaddr_in	m_addrSer, m_addrCli;

	int m_iLocalPort;
	bool m_bIsConnected;
	bool m_bSendPermission;//����ֻ�н��յ��ͻ�����������Ҫ���ͣ��ñ��������Ƿ������ݡ�
};