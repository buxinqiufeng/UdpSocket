#ifndef __UDPSERVER_H__
#define __UDPSERVER_H__
#include <iostream>
#include <sys/select.h>
#include <string.h>
#include <pthread.h>
using namespace std;

#include "../common/Common.h"
#include "ClientList.h"

typedef void (*pRecvMsgHandler)(string msg, int socket, int time);
typedef void (*pAcceptHandler)(string ip, string port, int handle);
typedef void (*pDisconnectHandler)(int socket);

class UdpServer
{
    struct MsgInfo
    {
        MsgInfo(){Initialize();}
        MsgInfo(int socket, string msg):Fd(socket), Content(msg){Initialize();}
        MsgInfo(int socket, const char* msg):Fd(socket){Initialize();string tmp=msg;Content=tmp;}
        long Time;
        int Fd;
        string Content;
    private:
        void Initialize()
        {
            time(&Time);
        }
    };
protected:
    UdpServer();
    virtual ~UdpServer();
    bool Initialize(const int port, unsigned long recvFunc);
    void SetCustomHandler(unsigned long acceptFunc, unsigned long disconnectFunc);
    bool SendData(const string &data, int socket);
    bool Uninitialize();
    void CloseSocket(int socket);
    void CloseAll();
protected:
    virtual void OnClientConnect(string ip, string port, int socket){};
    virtual void OnReceiveMessage(const string msg, const int socket, const int time){};
    virtual void OnClientDisconnect(int socket){};
private:
    static void *AcceptThread(void * pParam);
    static void * ManageThread(void * pParam);
    static void * OperatorThread(void * pParam);

    int mServerSocket;
    pRecvMsgHandler mOperaFunc;
    pAcceptHandler mAcceptFunc;
    pDisconnectHandler mDisconnectFunc;
    pthread_mutex_t mMutex;
    static int mMaxListen;
    pthread_t mAcceptThread;
    pthread_t mManageThread;
    fd_set mReadFd;
    list<MsgInfo> mDataList;
    list<int> mCliList;
};

#endif
