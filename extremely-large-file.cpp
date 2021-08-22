#include <Windows.h>
#include <iostream>
#include <time.h>


using namespace std;

int main()
{
	std::cout << "Hello World!\n";
	clock_t start, end;
	start = clock();

	// �����ļ�����
	HANDLE hFile1 = CreateFile(TEXT("D:\\data.dat"), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile1 == INVALID_HANDLE_VALUE)
	{
		cout << "�����ļ�1����ʧ�ܣ�������룺" << GetLastError() << endl;
		return 0;
	}
	HANDLE hFile2 = CreateFile(TEXT("D:\\data2.dat"), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile2 == INVALID_HANDLE_VALUE)
	{
		cout << "�����ļ�2����ʧ�ܣ�������룺" << GetLastError() << endl;
		return 0;
	}

	// �����ļ�ӳ�����
	HANDLE hFileMap1 = CreateFileMapping(hFile1, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hFileMap1 == NULL)
	{
		cout << "�����ļ�1map����ʧ�ܣ�������룺" << GetLastError() << endl;
		return 0;
	}
	HANDLE hFileMap2 = CreateFileMapping(hFile2, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hFileMap2 == NULL)
	{
		cout << "�����ļ�2map����ʧ�ܣ�������룺" << GetLastError() << endl;
		return 0;
	}

	// �õ�ϵͳ��������
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	DWORD dwGran = SysInfo.dwAllocationGranularity;

	// �õ��ļ��ߴ�
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hFile1, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);

	// �ر��ļ�����
	CloseHandle(hFile1);
	CloseHandle(hFile2);
	// ƫ�Ƶ�ַ 
	// �趨��С��ƫ�����Ȳ���
	// 0x40000000 == 1   GB
	// 0x20000000 == 512 MB
	// 0x10000000 == 256 MB
	// 0x8000000  == 128 MB
	// 0x4000000  == 64  MB
	// 0x2000000  == 32  MB
	// 0x1000000  == 16  MB
	// 0x800000   == 8   MB
	// 0x100000   == 1   MB
	__int64 qwFileOffset = 0;
	// ���С
	DWORD dwBlockBytes = dwGran;
	if (qwFileSize < dwGran)
		dwBlockBytes = (DWORD)qwFileSize;
	while (qwFileSize > 0)
	{
		// ӳ����ͼ
		LPBYTE lpbMapAddress = (LPBYTE)MapViewOfFile(hFileMap1, FILE_MAP_ALL_ACCESS,
			(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF),
			dwBlockBytes);
		if (lpbMapAddress == NULL)
		{
			cout << "map�ļ�����ʧ�ܣ�������룺" << GetLastError() << endl;
			return 0;
		}
		// ��ӳ�����ͼ���з���
		for (DWORD i = 0; i < dwBlockBytes; i++)
			BYTE temp = *(lpbMapAddress + i);
		// �����ļ�ӳ��
		UnmapViewOfFile(lpbMapAddress);
		// ��������
		qwFileOffset += dwBlockBytes;
		qwFileSize -= dwBlockBytes;
	}

	// �ر��ļ�ӳ�������
	CloseHandle(hFileMap1);
	cout << "ok" << endl;


	// �����ļ��ں˶�������������hFile
	HANDLE hFile = CreateFile(TEXT("D:\\data.dat"),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	// �����ļ�ӳ���ں˶��󣬾��������hFileMapping
	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE,
		0, 0x4000000, NULL);
	// �ͷ��ļ��ں˶���
	CloseHandle(hFile);

	__int64 qwFileSize = 0x4000000;  //64 MB
	__int64 qwFileOffset = 0;
	DWORD dwBytesInBlock = 1000 * SysInfo.dwAllocationGranularity;
	// ���ļ�����ӳ�䵽���̵ĵ�ַ�ռ�
	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF), dwBytesInBlock);

	// ��������һ���ļ��ں˶���
	HANDLE hFile2 = CreateFile(TEXT("D:\\data.dat"),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	// ��ʵ�����ݳ��ȴ�������һ���ļ�ӳ���ں˶���
	HANDLE hFileMapping2 = CreateFileMapping(hFile2,
		NULL,
		PAGE_READWRITE,
		0,
		(DWORD)(qwFileOffset & 0xFFFFFFFF),
		NULL);
	// �ر��ļ��ں˶���
	CloseHandle(hFile2);
	// ���ļ�����ӳ�䵽���̵ĵ�ַ�ռ�
	PBYTE pbFile2 = (PBYTE)MapViewOfFile(hFileMapping2,
		FILE_MAP_ALL_ACCESS,
		0, 0, qwFileOffset);
	// �����ݴ�ԭ�����ڴ�ӳ���ļ����Ƶ����ڴ�ӳ���ļ�
	memcpy(pbFile2, pbFile, qwFileOffset);
	//�ӽ��̵ĵ�ַ�ռ䳷���ļ�����ӳ��
	UnmapViewOfFile(pbFile);
	UnmapViewOfFile(pbFile2);
	// �ر��ļ�ӳ�����
	CloseHandle(hFileMapping);
	CloseHandle(hFileMapping2);



	end = clock();
	cout << "��ʱ��" << (end - start) / CLOCKS_PER_SEC << "s" << endl;
	system("pause");
	return 0;
}