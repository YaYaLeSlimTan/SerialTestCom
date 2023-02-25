#pragma once
#include <vector>

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#define SERIAL_CODE_MAX_LEN      0x20

using namespace std;
class CSerialBase
{
public:
	CSerialBase();
	virtual ~CSerialBase();


	BOOL Open(int nPort, int nBaud);
	BOOL Close(void);

	int ReadData(vector<char> &recvArray);	//串口接受数据

	BOOL IsOpened(void) { return(m_bOpened); }
	int SendData(int addr, int funcIndex, vector<char>& sendArray);
protected:	
	int SendDataReal(vector<char> &sendArray);//串口发送数据

	BOOL Open(const TCHAR* nPort, int nBaud);


protected:

	HANDLE m_hIDComDev;
	OVERLAPPED m_OverlappedRead;
	OVERLAPPED m_OverlappedWrite;
	BOOL m_bOpened;

};

