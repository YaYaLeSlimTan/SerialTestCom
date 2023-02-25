#include "pch.h"
#include "SerialWithCrc.h"

CSerialWithCrc::CSerialWithCrc()
{

}
CSerialWithCrc::~CSerialWithCrc()
{

}

int CSerialWithCrc::SendData(int addr, int funcIndex, vector<char>& sendArray)  //在此数据基础上添加crc
{
	UINT16 crc = 0;
	vector<char> realSendArray;
	realSendArray.push_back(addr);
	realSendArray.push_back(funcIndex);
	for (int i = 0; i < sendArray.size(); ++i)	//这是原来的数据，不包含CRC16
	{
		realSendArray.push_back(sendArray[i]);
	}
	crc = Crc16((BYTE*)&realSendArray[0], realSendArray.size());//crc16算结果
	realSendArray.push_back(crc & 0xff);//低位
	realSendArray.push_back(crc >> 8);	//高位
	return CSerialBase::SendData(addr, funcIndex, realSendArray);
}