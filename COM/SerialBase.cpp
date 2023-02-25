#include "pch.h"
#include "SerialBase.h"
//开关宏以实现奇校验或偶校验位
//#define _COM_CHECKSUM_ODD_PARITY_	//奇校验
//#define _COM_CHECKSUM_EVEN_PARITY_	//偶校验

#ifdef _COM_CHECKSUM_ODD_PARITY_
#define _COM_CHECKSUM_ODD_
#else
#undef _COM_CHESUM_ODD_
#endif // _COM_CHECKSUM_ODD_PARITY_

#ifdef _COM_CHECKSUM_EVEN_PARITY_
#define _COM_CHECKSUM_EVEN_
#else
#undef _COM_CHECKSUM_EVEN_
#endif // _COM_CHECKSUM_EVEN_PARITY_

CSerialBase::CSerialBase()
{
	memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));
	memset(&m_OverlappedWrite, 0, sizeof(OVERLAPPED));
	m_hIDComDev = INVALID_HANDLE_VALUE;
	m_bOpened = FALSE;
}


CSerialBase::~CSerialBase()
{
	Close();
}


BOOL CSerialBase::Open(int nPort, int nBaud)
{
	if (m_bOpened) return(TRUE);
	TCHAR szPort[15];
	wsprintf(szPort, _TEXT("COM%d"), nPort);	//转端口号变成字符串
	return Open(szPort, nBaud);
}



BOOL CSerialBase::Open(const TCHAR* szPort, int nBaud)
{
	if (m_bOpened) return(TRUE);

	//TCHAR szComParams[50];
	//调用API打开串口
	m_hIDComDev = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (m_hIDComDev == INVALID_HANDLE_VALUE)
		m_hIDComDev = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hIDComDev == INVALID_HANDLE_VALUE)
		return(FALSE);

	//两个重叠结构，用来串口异步读写
	memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));
	memset(&m_OverlappedWrite, 0, sizeof(OVERLAPPED));

	//串口的超时参数调置
	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(m_hIDComDev, &CommTimeOuts);	//API，设定串口超时


	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hIDComDev, &dcb);	//获取串口参数
	dcb.BaudRate = nBaud;	//波特率
	dcb.ByteSize = 8;	//字节数

	dcb.Parity = 0;	//默认无校验
#ifdef _COM_CHECKSUM_EVEN_
	dcb.Parity = 2;        /* 0-4=None,Odd,Even,Mark,Space    *///偶校验位
#endif // _COM_CHECKSUM_EVEN_

#ifdef _COM_CHECKSUM_ODD_
	dcb.Parity = 1;		//奇校验位
#endif // _COM_CHECKSUM_ODD_

	unsigned char ucSet;	//硬件应答位
	ucSet = (unsigned char)((FC_RTSCTS & FC_DTRDSR) != 0);
	ucSet = (unsigned char)((FC_RTSCTS & FC_RTSCTS) != 0);
	ucSet = (unsigned char)((FC_RTSCTS & FC_XONXOFF) != 0);

	DWORD dwError = 0;
	if (!SetCommState(m_hIDComDev, &dcb))	//设定串口参数
	{
		dwError = GetLastError();
		wchar_t buf[128] = { 0 };
		swprintf_s(buf, sizeof(buf), _T("error 1 = %d"), dwError);
		AfxMessageBox(buf, MB_OK);
		return FALSE;
	}

	if (!SetupComm(m_hIDComDev, 10000, 10000))	//设定串口最大最小队列缓冲区
	{
		dwError = GetLastError();
		wchar_t buf[128] = { 0 };
		swprintf_s(buf, sizeof(buf), _T("error 2 = %d"), dwError);
		AfxMessageBox(buf, MB_OK);
		return FALSE;
	}

	//重叠结构的事件
	m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_OverlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_OverlappedRead.hEvent == NULL ||
		m_OverlappedWrite.hEvent == NULL)
	{
		dwError = GetLastError();
		wchar_t buf[128] = { 0 };
		swprintf_s(buf, sizeof(buf), _T("error 3 = %d"), dwError);
		AfxMessageBox(buf, MB_OK);
		return FALSE;
	}
	m_bOpened = TRUE;

	return(m_bOpened);

}

BOOL CSerialBase::Close(void)
{
	if (!m_bOpened || m_hIDComDev == INVALID_HANDLE_VALUE) return(TRUE);

	if (m_OverlappedRead.hEvent != NULL) CloseHandle(m_OverlappedRead.hEvent);
	if (m_OverlappedWrite.hEvent != NULL) CloseHandle(m_OverlappedWrite.hEvent);
	CloseHandle(m_hIDComDev);
	m_bOpened = FALSE;
	m_hIDComDev = INVALID_HANDLE_VALUE;
	return(TRUE);
}


int CSerialBase::ReadData(vector<char>& recvArray)	//串口接受数据
{
	int len = 0;
	if (!m_bOpened || m_hIDComDev == INVALID_HANDLE_VALUE) 
		return(0);

	BOOL bReadStatus;
	DWORD dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	//清除串口错误
	ClearCommError(m_hIDComDev, &dwErrorFlags, &ComStat);
	int err = GetLastError();
	if (!ComStat.cbInQue) 
		return(0);

	dwBytesRead = (DWORD)ComStat.cbInQue;
	char *buffer= new char[dwBytesRead];	

	//真正读串口的API
	bReadStatus = ReadFile(m_hIDComDev, buffer, dwBytesRead, &dwBytesRead, &m_OverlappedRead);
	if (!bReadStatus) {
		if (GetLastError() == ERROR_IO_PENDING) {
			len=(int)dwBytesRead;
		}
		len = 0;
	}
	len = dwBytesRead;

	for (int i = 0; i < len; ++i)
	{
		recvArray.push_back(buffer[i]);
	}
	delete[]buffer;
	return len;
}
int CSerialBase::SendDataReal(vector<char>& sendArray)//串口发送数据
{
	int len = 0;
	int sendLength = sendArray.size();
	BOOL bWriteStat = FALSE;
	DWORD dwBytesWritten = 0;
	int tryTimes = 0;
	while (IsOpened() && len < sendLength)
	{
		//真正发送数据API
		bWriteStat = WriteFile(m_hIDComDev, (char*)&sendArray.at(0), sendLength, &dwBytesWritten, &m_OverlappedWrite);
		if (!bWriteStat && (GetLastError() == ERROR_IO_PENDING)) //发数据挂起状态，需要等一段时间
		{
			if (WaitForSingleObject(m_OverlappedWrite.hEvent, 20))
			{
				++tryTimes;
				dwBytesWritten = 0;
			}
			else {
				GetOverlappedResult(m_hIDComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE);//获取真正的发送完的字节数
				m_OverlappedWrite.Offset += dwBytesWritten;
			}
		}
		len += dwBytesWritten;
		if (tryTimes > 10)	//超过尝试次数，清空发送缓存
		{
			PurgeComm(m_hIDComDev, PURGE_TXABORT | PURGE_TXCLEAR);//清空串口的发送缓存
			break;
		}
	}


	return len;

}

int CSerialBase::SendData(int addr, int funcIndex, vector<char>& sendArray)
{
	int sendLength = 0;
	sendLength = SendDataReal(sendArray);
	return sendLength;
}