#include "helper/part_helper.h"

#include "part01/part01_class01.h"
#include "part01/part01_class02.h"
#include "part01/part01_class03.h"

void CPartHelper::excuteClass01()
{

}

void CPartHelper::excuteClass02()
{
	CPart01Class02 test_main;
	test_main.prove01();
	SScreenPhysicsSize master;
	SScreenPhysicsSize slave;
	test_main.getScreenScreenPhysicsSize(master, slave);
	test_main.getIntArrByteNum();
	test_main.removeDirAllFile(L"D:/res");

	bool ret = test_main.CopyFolder(L"D:/test_utils4/", L"D:/test_log/");
	test_main.testMonitorFromWindow();
	test_main.testLPCSTR2String();
	string res = "";

}

void CPartHelper::excuteClass03()
{
	CPart01Class03 test_main;
	test_main.testGetIpStrHeader();
}
