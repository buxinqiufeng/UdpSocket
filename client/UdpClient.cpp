#include "UdpClient.h"

UdpClient::UdpClient()
{
    mConnectHdlr = 0;
    mMsgHdlr = 0;
    mDisconHdlr = 0;
    mSocket = 0;
    mRecvThread = 0;
    mDataThread = 0;
    mDataList = 0;
}

UdpClient::Initialize(const string ip, const string port)
{

}

UdpClient::SetCustomHandler(unsigned long connectHdlr, unsigned long msgHdlr, unsigned long disconnectHdlr)
{

}

UdpClient::SendData(const string &data)
{

}
