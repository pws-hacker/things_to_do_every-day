#include <string>

using namespace std;

class CPart01Class02
{
public:
	CPart01Class02();
	~CPart01Class02();

	// ����1��������̬��Ա�������޸ı���ֵ�ľ�̬�������Ƿ��Ժ�ÿ��ʵ����ֵ���Ǹı���ֵ
	// ʵ��֤������̬��Ա�����;�̬��Ա���� �Ѿ��洢����̬�洢������ϵ��ʵ���Ѿ������������Ա��
	// ����ÿ��ʵ�����ɷ��������̬��Ա������ֵ��һ�����·������๫������һ��
	static void setStaticVariable(int val);
	int			getStaticVariable();
	void		setName(string name);



public:
	string		m_sName;
private:
	static int  m_iTestVariable;

};

class CPart01Class02Test
{
public:
	CPart01Class02Test() {};
	~CPart01Class02Test() {};

	void				prove01();
private:
	CPart01Class02		m_cTest01;
	CPart01Class02		m_cTest02;
};
