#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__
#include <iostream>
#include <list>
#include <pthread.h>
using namespace std;

#include "ClientInfo.hpp"

typedef void (*pConnectHandler)();
typedef void (*pMessageHandler)(void *param);
typedef void (*pDisconnectHandler)(void *param);
class UdpClient
{
public:
    UdpClient();
    bool Initialize(const string ip, const int port);
    void SetCustomHandler(unsigned long connectHdlr, unsigned long msgHdlr, unsigned long disconnectHdlr);
    bool SendData(const string &data);

    virtual void OnConnected(){};
    virtual void OnDisconnect(){};
    virtual void OnRecvMessage(const string msg){};

private:
    static void *RecvThread(void * pParam);
    static void *DataThread(void * pParam);
private:
    pConnectHandler mConnectHdlr;
    pMessageHandler mMsgHdlr;
    pDisconnectHandler mDisconHdlr;
    int mSocketFd;

    pthread_mutex_t mMutex;
    pthread_t mRecvThread;
    pthread_t mDataThread;

    list<string> mDataList;
    int mMaxMsgLen;
};

#endif
