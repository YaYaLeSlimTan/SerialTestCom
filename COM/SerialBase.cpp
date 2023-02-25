#include "pch.h"
#include "SerialBase.h"
//���غ���ʵ����У���żУ��λ
//#define _COM_CHECKSUM_ODD_PARITY_	//��У��
//#define _COM_CHECKSUM_EVEN_PARITY_	//żУ��

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
	wsprintf(szPort, _TEXT("COM%d"), nPort);	//ת�˿ںű���ַ���
	return Open(szPort, nBaud);
}



BOOL CSerialBase::Open(const TCHAR* szPort, int nBaud)
{
	if (m_bOpened) return(TRUE);

	//TCHAR szComParams[50];
	//����API�򿪴���
	m_hIDComDev = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (m_hIDComDev == INVALID_HANDLE_VALUE)
		m_hIDComDev = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hIDComDev == INVALID_HANDLE_VALUE)
		return(FALSE);

	//�����ص��ṹ�����������첽��д
	memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));
	memset(&m_OverlappedWrite, 0, sizeof(OVERLAPPED));

	//���ڵĳ�ʱ��������
	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(m_hIDComDev, &CommTimeOuts);	//API���趨���ڳ�ʱ


	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hIDComDev, &dcb);	//��ȡ���ڲ���
	dcb.BaudRate = nBaud;	//������
	dcb.ByteSize = 8;	//�ֽ���

	dcb.Parity = 0;	//Ĭ����У��
#ifdef _COM_CHECKSUM_EVEN_
	dcb.Parity = 2;        /* 0-4=None,Odd,Even,Mark,Space    *///żУ��λ
#endif // _COM_CHECKSUM_EVEN_

#ifdef _COM_CHECKSUM_ODD_
	dcb.Parity = 1;		//��У��λ
#endif // _COM_CHECKSUM_ODD_

	unsigned char ucSet;	//Ӳ��Ӧ��λ
	ucSet = (unsigned char)((FC_RTSCTS & FC_DTRDSR) != 0);
	ucSet = (unsigned char)((FC_RTSCTS & FC_RTSCTS) != 0);
	ucSet = (unsigned char)((FC_RTSCTS & FC_XONXOFF) != 0);

	DWORD dwError = 0;
	if (!SetCommState(m_hIDComDev, &dcb))	//�趨���ڲ���
	{
		dwError = GetLastError();
		wchar_t buf[128] = { 0 };
		swprintf_s(buf, sizeof(buf), _T("error 1 = %d"), dwError);
		AfxMessageBox(buf, MB_OK);
		return FALSE;
	}

	if (!SetupComm(m_hIDComDev, 10000, 10000))	//�趨���������С���л�����
	{
		dwError = GetLastError();
		wchar_t buf[128] = { 0 };
		swprintf_s(buf, sizeof(buf), _T("error 2 = %d"), dwError);
		AfxMessageBox(buf, MB_OK);
		return FALSE;
	}

	//�ص��ṹ���¼�
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


int CSerialBase::ReadData(vector<char>& recvArray)	//���ڽ�������
{
	int len = 0;
	if (!m_bOpened || m_hIDComDev == INVALID_HANDLE_VALUE) 
		return(0);

	BOOL bReadStatus;
	DWORD dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	//������ڴ���
	ClearCommError(m_hIDComDev, &dwErrorFlags, &ComStat);
	int err = GetLastError();
	if (!ComStat.cbInQue) 
		return(0);

	dwBytesRead = (DWORD)ComStat.cbInQue;
	char *buffer= new char[dwBytesRead];	

	//���������ڵ�API
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
int CSerialBase::SendDataReal(vector<char>& sendArray)//���ڷ�������
{
	int len = 0;
	int sendLength = sendArray.size();
	BOOL bWriteStat = FALSE;
	DWORD dwBytesWritten = 0;
	int tryTimes = 0;
	while (IsOpened() && len < sendLength)
	{
		//������������API
		bWriteStat = WriteFile(m_hIDComDev, (char*)&sendArray.at(0), sendLength, &dwBytesWritten, &m_OverlappedWrite);
		if (!bWriteStat && (GetLastError() == ERROR_IO_PENDING)) //�����ݹ���״̬����Ҫ��һ��ʱ��
		{
			if (WaitForSingleObject(m_OverlappedWrite.hEvent, 20))
			{
				++tryTimes;
				dwBytesWritten = 0;
			}
			else {
				GetOverlappedResult(m_hIDComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE);//��ȡ�����ķ�������ֽ���
				m_OverlappedWrite.Offset += dwBytesWritten;
			}
		}
		len += dwBytesWritten;
		if (tryTimes > 10)	//�������Դ�������շ��ͻ���
		{
			PurgeComm(m_hIDComDev, PURGE_TXABORT | PURGE_TXCLEAR);//��մ��ڵķ��ͻ���
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