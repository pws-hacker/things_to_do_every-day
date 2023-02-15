#include "part01/part01_class02.h"
#include <iostream>
#include <Windows.h>
#include <SetupApi.h>
#include <atlstr.h>
#pragma comment(lib, "setupapi.lib")
int  CPart01Class02Test::m_iTestVariable = 0;
CPart01Class02Test::CPart01Class02Test()
{

}

CPart01Class02Test::~CPart01Class02Test()
{
}

void CPart01Class02Test::setStaticVariable(int val)
{
	m_iTestVariable = val;
}

int CPart01Class02Test::getStaticVariable()
{
	return m_iTestVariable;
}

void CPart01Class02Test::setName(string name)
{
	m_sName = name;
}
const GUID GUID_DEVINTERFACE_MONITOR = { 0xE6F07B5F , 0xEE97 , 0x4a90 , 0xB076 , 0x33F57BF4EAA7 };
const GUID GUID_CLASS_MONITOR = { 0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 };
#define STATUS_BUFFER_TOO_SMALL 0xC0000023
#define STATUS_SUCCESS	0x00000000
std::wstring GetKeyPathFromHKEY(HKEY key)
{
	std::wstring keyPath;

	if (key != NULL)
	{
		HMODULE dll = LoadLibrary(L"ntdll.dll");

		if (dll != NULL) {
			typedef DWORD(__stdcall* NtQueryKeyType)(
				HANDLE  KeyHandle,
				int KeyInformationClass,
				PVOID  KeyInformation,
				ULONG  Length,
				PULONG  ResultLength);

			NtQueryKeyType func = reinterpret_cast<NtQueryKeyType>(::GetProcAddress(dll, "NtQueryKey"));

			if (func != NULL) {
				DWORD size = 0;
				DWORD result = 0;
				result = func(key, 3, 0, 0, &size);

				if (result == STATUS_BUFFER_TOO_SMALL)
				{
					size = size + 2;
					wchar_t* buffer = new (std::nothrow) wchar_t[size / sizeof(wchar_t)]; // size is in bytes

					if (buffer != NULL)
					{
						result = func(key, 3, buffer, size, &size);

						if (result == STATUS_SUCCESS)
						{
							buffer[size / sizeof(wchar_t)] = L'\0';
							keyPath = std::wstring(buffer + 2);
						}

						delete[] buffer;
					}
				}
			}

			FreeLibrary(dll);
		}
	}

	return keyPath;
}
#define NAME_SIZE 128
bool GetMonitorSizeFromEDID(const HKEY hDevRegKey, short& WidthMm, short& HeightMm)
{
	DWORD dwType, AcutalValueNameLength = NAME_SIZE;
	TCHAR valueName[NAME_SIZE];
	BYTE EDIDdata[1024];
	DWORD edidsize = sizeof(EDIDdata);

	for (LONG i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; ++i)
	{
		retValue = RegEnumValue(hDevRegKey, i, &valueName[0],
			&AcutalValueNameLength, NULL, &dwType,
			EDIDdata, // buffer
			&edidsize); // buffer size

		if (retValue != ERROR_SUCCESS || 0 != _tcscmp(valueName, _T("EDID")))
			continue;

		WidthMm = ((EDIDdata[68] & 0xF0) << 4) + EDIDdata[66];
		HeightMm = ((EDIDdata[68] & 0x0F) << 8) + EDIDdata[67];

		return true; // valid EDID found
	}

	return false; // EDID not found
}
bool GetSizeForDevID(const std::wstring& TargetDevID, short& WidthMm, short& HeightMm)
{
	HDEVINFO devInfo = SetupDiGetClassDevsEx(
		&GUID_CLASS_MONITOR, //class GUID
		NULL, //enumerator
		NULL, //HWND
		DIGCF_PRESENT, // Flags //DIGCF_ALLCLASSES|
		NULL, // device info, create a new one.
		NULL, // machine name, local machine
		NULL);// reserved

	if (NULL == devInfo)
		return false;

	bool bRes = false;

	for (ULONG i = 0; ERROR_NO_MORE_ITEMS != GetLastError(); ++i)
	{
		SP_DEVINFO_DATA devInfoData;
		memset(&devInfoData, 0, sizeof(devInfoData));
		devInfoData.cbSize = sizeof(devInfoData);

		if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData))
		{
			HKEY hDevRegKey = SetupDiOpenDevRegKey(devInfo, &devInfoData,
				DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

			if (!hDevRegKey || (hDevRegKey == INVALID_HANDLE_VALUE))
				continue;
			std::wstring fullRegistryKey = GetKeyPathFromHKEY(hDevRegKey);
			if (fullRegistryKey.find(TargetDevID) != string::npos)
				bRes = GetMonitorSizeFromEDID(hDevRegKey, WidthMm, HeightMm);
			RegCloseKey(hDevRegKey);
		}
	}
	SetupDiDestroyDeviceInfoList(devInfo);
	return bRes;
}

void CPart01Class02::getScreenScreenPhysicsSize(SScreenPhysicsSize& master, SScreenPhysicsSize& slave)
{
	int count = GetSystemMetrics(SM_CMONITORS);
	HDC hdc = GetDC(NULL);
	int hor = GetDeviceCaps(hdc, HORZSIZE);
	int ver = GetDeviceCaps(hdc, VERTSIZE);
	float cx = (float)GetSystemMetrics(SM_CXSCREEN);
	float cy = (float)GetSystemMetrics(SM_CYSCREEN);
	float dpmX = cx / (float)hor;
	float dpmY = cy / (float)ver;

	DWORD startTime = GetTickCount();
	bool deviceFound = false;
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	int i = 0;
	DWORD dev = 0; // device index
	int id = 1; // monitor number, as used by Display Properties > Settings

	std::wstring DeviceID;
	bool bFoundDevice = false;

	while (EnumDisplayDevices(0, dev, &dd, 0))
	{
		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD devMon = 0;

		// 主屏
		if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP && !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
		{
			while (EnumDisplayDevices(dd.DeviceName, devMon, &ddMon, 0))
			{
				DeviceID = ddMon.DeviceID;
				DeviceID = DeviceID.substr(8, DeviceID.find('\\', 9) - 8);
				short WidthMm = 0, HeightMm = 0;
				bFoundDevice = GetSizeForDevID(DeviceID, WidthMm, HeightMm);
				deviceFound = bFoundDevice;
				devMon++;
				i++;

				ZeroMemory(&ddMon, sizeof(ddMon));
				ddMon.cb = sizeof(ddMon);
			}

			ZeroMemory(&dd, sizeof(dd));
			dd.cb = sizeof(dd);
		}
		// 从屏
		if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP && !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && !(dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
		{
			while (EnumDisplayDevices(dd.DeviceName, devMon, &ddMon, 0) )
			{
				DeviceID = ddMon.DeviceID;
				DeviceID = DeviceID.substr(8, DeviceID.find('\\', 9) - 8);
				short WidthMm = 0, HeightMm = 0;
				bFoundDevice = GetSizeForDevID(DeviceID, WidthMm, HeightMm);
				deviceFound = bFoundDevice;
				devMon++;
				i++;

				ZeroMemory(&ddMon, sizeof(ddMon));
				ddMon.cb = sizeof(ddMon);
			}

			ZeroMemory(&dd, sizeof(dd));
			dd.cb = sizeof(dd);
		}

		
		dev++;
		i++;
	}
	DWORD continueTime = 0;
	DWORD endTime = GetTickCount();

	continueTime = endTime - startTime;

}

void CPart01Class02::getIntArrByteNum()
{
	int arr[111];
	cout << "结论：int arr[111] 占用字节数 = " << (int)sizeof(arr) << endl;
}

void CPart01Class02::prove01()
{
	m_cTest01.setName("test01");
	int ret_num = m_cTest01.getStaticVariable();
	CPart01Class02Test::setStaticVariable(10);
	m_cTest02.setName("test02");
	int ret_num1 = m_cTest01.getStaticVariable();
	int ret_num2 = m_cTest02.getStaticVariable();
	cout << "ret_num1 = " << ret_num1 << ",ret_num2=" << ret_num2 << ",结论：静态变量为 所有类实例公共变量";
}
