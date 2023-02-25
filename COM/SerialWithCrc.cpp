#include "pch.h"
#include "SerialWithCrc.h"

CSerialWithCrc::CSerialWithCrc()
{

}
CSerialWithCrc::~CSerialWithCrc()
{

}

int CSerialWithCrc::SendData(int addr, int funcIndex, vector<char>& sendArray)  //�ڴ����ݻ��������crc
{
	UINT16 crc = 0;
	vector<char> realSendArray;
	realSendArray.push_back(addr);
	realSendArray.push_back(funcIndex);
	for (int i = 0; i < sendArray.size(); ++i)	//����ԭ�������ݣ�������CRC16
	{
		realSendArray.push_back(sendArray[i]);
	}
	crc = Crc16((BYTE*)&realSendArray[0], realSendArray.size());//crc16����
	realSendArray.push_back(crc & 0xff);//��λ
	realSendArray.push_back(crc >> 8);	//��λ
	return CSerialBase::SendData(addr, funcIndex, realSendArray);
}