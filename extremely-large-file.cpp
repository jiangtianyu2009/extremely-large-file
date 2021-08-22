#include <Windows.h>
#include <iostream>
#include <time.h>


using namespace std;

int main()
{
	std::cout << "Hello World!\n";
	clock_t start, end;
	start = clock();

	// 创建文件对象
	HANDLE hFile1 = CreateFile(TEXT("D:\\data.dat"), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile1 == INVALID_HANDLE_VALUE)
	{
		cout << "创建文件1对象失败，错误代码：" << GetLastError() << endl;
		return 0;
	}
	HANDLE hFile2 = CreateFile(TEXT("D:\\data2.dat"), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile2 == INVALID_HANDLE_VALUE)
	{
		cout << "创建文件2对象失败，错误代码：" << GetLastError() << endl;
		return 0;
	}

	// 创建文件映射对象
	HANDLE hFileMap1 = CreateFileMapping(hFile1, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hFileMap1 == NULL)
	{
		cout << "创建文件1map对象失败，错误代码：" << GetLastError() << endl;
		return 0;
	}
	HANDLE hFileMap2 = CreateFileMapping(hFile2, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hFileMap2 == NULL)
	{
		cout << "创建文件2map对象失败，错误代码：" << GetLastError() << endl;
		return 0;
	}

	// 得到系统分配粒度
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	DWORD dwGran = SysInfo.dwAllocationGranularity;

	// 得到文件尺寸
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hFile1, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);

	// 关闭文件对象
	CloseHandle(hFile1);
	CloseHandle(hFile2);
	// 偏移地址 
	// 设定大小、偏移量等参数
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
	// 块大小
	DWORD dwBlockBytes = dwGran;
	if (qwFileSize < dwGran)
		dwBlockBytes = (DWORD)qwFileSize;
	while (qwFileSize > 0)
	{
		// 映射视图
		LPBYTE lpbMapAddress = (LPBYTE)MapViewOfFile(hFileMap1, FILE_MAP_ALL_ACCESS,
			(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF),
			dwBlockBytes);
		if (lpbMapAddress == NULL)
		{
			cout << "map文件对象失败，错误代码：" << GetLastError() << endl;
			return 0;
		}
		// 对映射的视图进行访问
		for (DWORD i = 0; i < dwBlockBytes; i++)
			BYTE temp = *(lpbMapAddress + i);
		// 撤消文件映像
		UnmapViewOfFile(lpbMapAddress);
		// 修正参数
		qwFileOffset += dwBlockBytes;
		qwFileSize -= dwBlockBytes;
	}

	// 关闭文件映射对象句柄
	CloseHandle(hFileMap1);
	cout << "ok" << endl;


	// 创建文件内核对象，其句柄保存于hFile
	HANDLE hFile = CreateFile(TEXT("D:\\data.dat"),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	// 创建文件映射内核对象，句柄保存于hFileMapping
	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE,
		0, 0x4000000, NULL);
	// 释放文件内核对象
	CloseHandle(hFile);

	__int64 qwFileSize = 0x4000000;  //64 MB
	__int64 qwFileOffset = 0;
	DWORD dwBytesInBlock = 1000 * SysInfo.dwAllocationGranularity;
	// 将文件数据映射到进程的地址空间
	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF), dwBytesInBlock);

	// 创建另外一个文件内核对象
	HANDLE hFile2 = CreateFile(TEXT("D:\\data.dat"),
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	// 以实际数据长度创建另外一个文件映射内核对象
	HANDLE hFileMapping2 = CreateFileMapping(hFile2,
		NULL,
		PAGE_READWRITE,
		0,
		(DWORD)(qwFileOffset & 0xFFFFFFFF),
		NULL);
	// 关闭文件内核对象
	CloseHandle(hFile2);
	// 将文件数据映射到进程的地址空间
	PBYTE pbFile2 = (PBYTE)MapViewOfFile(hFileMapping2,
		FILE_MAP_ALL_ACCESS,
		0, 0, qwFileOffset);
	// 将数据从原来的内存映射文件复制到此内存映射文件
	memcpy(pbFile2, pbFile, qwFileOffset);
	//从进程的地址空间撤消文件数据映像
	UnmapViewOfFile(pbFile);
	UnmapViewOfFile(pbFile2);
	// 关闭文件映射对象
	CloseHandle(hFileMapping);
	CloseHandle(hFileMapping2);



	end = clock();
	cout << "耗时：" << (end - start) / CLOCKS_PER_SEC << "s" << endl;
	system("pause");
	return 0;
}