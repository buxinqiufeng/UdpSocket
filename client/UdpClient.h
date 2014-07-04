#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__
#include <iostream>
#include <list>
#include <pthread.h>
using namespace std;

#include "../common/ClientInfo.hpp"

typedef void (*pConnectHandler)(void *param);
typedef void (*pMessageHandler)(void *param);
typedef void (*pDisconnectHandler)(void *param);
class UdpClient
{
public:
    UdpClient();
    bool Initialize(const string ip, const string port);
    void SetCustomHandler(unsigned long connectHdlr, unsigned long msgHdlr, unsigned long disconnectHdlr);
    bool SendData(const string &data);

private:
    pConnectHandler mConnectHdlr;
    pMessageHandler mMsgHdlr;
    pDisconnectHandler mDisconHdlr;
    int mSocket;
    pthread_t mRecvThread;
    pthread_t mDataThread;
    list<string> mDataList;
}

#endif
