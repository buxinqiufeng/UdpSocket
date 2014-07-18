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
    ~UdpClient();
    void SetCustomHandler(unsigned long connectHdlr, unsigned long msgHdlr, unsigned long disconnectHdlr);
    bool SendData(const string &data);
    void Disconnect();

protected:
    bool Initialize(const string ip, const int port);
    virtual void OnConnected(){};
    virtual void OnDisconnect(){};
    virtual void OnRecvMessage(const string msg){};

private:
    void DeInit();
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
    bool mIsRecvThreadRunning;
    bool mIsDataThreadRunnung;

    list<string> mDataList;
    int mMaxMsgLen;
    bool mIsConnecting;
};

#endif
