#pragma once
#include "SerialBase.h"
#include "ymbcrc.h"
class CSerialWithCrc :
    public CSerialBase
{
public:
    CSerialWithCrc();
    ~CSerialWithCrc();

    int SendData(int addr,int funcIndex,vector<char>& sendArray);  //在此数据基础上添加crc
private:


};

