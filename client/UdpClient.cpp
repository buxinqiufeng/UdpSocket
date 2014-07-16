
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <list>
using namespace std;

#include "../common/Common.h"
#include "UdpClient.h"

UdpClient::UdpClient()
{
    mConnectHdlr = 0;
    mMsgHdlr = 0;
    mDisconHdlr = 0;
    mSocketFd = 0;
    mRecvThread = 0;
    mDataThread = 0;
    mDataList.empty();
    mMaxMsgLen = 1024;
}

bool UdpClient::Initialize(const string ip, const int port)
{
    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == mSocketFd)
	{
		cerr << "socket init fail!" << endl;
		return false;
	}

	struct sockaddr_in s_add;
	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr= inet_addr(ip.c_str());
    s_add.sin_port=htons(port);

    if(-1 == connect(mSocketFd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        cerr << "Connect to server failed." << endl;
    }
    if(null != mConnectHdlr) mConnectHdlr();
    OnConnected();


    if(0 != pthread_create(&mRecvThread, NULL, RecvThread, this))
        return false;

    if(0 != pthread_create(&mDataThread, NULL, RecvThread, this))
        return false;

    return true;
}

void UdpClient::SetCustomHandler(unsigned long connectHdlr, unsigned long msgHdlr, unsigned long disconnectHdlr)
{
    mConnectHdlr = (pConnectHandler)connectHdlr;
    mMsgHdlr = (pMessageHandler)msgHdlr;
    mDisconHdlr = (pDisconnectHandler)disconnectHdlr;
}

bool UdpClient::SendData(const string &data)
{
    return false;
}

void *UdpClient::RecvThread(void * pParam)
{
    if(null == pParam) return 0;

    UdpClient *pThis = (UdpClient*)pParam;
    char *tmpBuf = new char(pThis->mMaxMsgLen);

    int res = recv(pThis->mSocketFd, tmpBuf, pThis->mMaxMsgLen, 0);
    if(0 < res)
    {
        pthread_mutex_lock(&pThis->mMutex);
        string tmp = tmpBuf;
        pThis->mDataList.push_back(tmp);
        pthread_mutex_unlock(&pThis->mMutex);
    }
    else if(0 == res)
    {
        cerr << "lost connect" << endl;
        pthread_mutex_lock(&pThis->mMutex);
        pThis->OnDisconnect();
        pthread_mutex_unlock(&pThis->mMutex);
    }
    else
    {
        cerr << "receive failed." << endl;
    }

    delete tmpBuf;
    return 0;
}

void *UdpClient::DataThread(void * pParam)
{
    if(null == pParam) return 0;

    UdpClient *pThis = (UdpClient*)pParam;
    while(1)
    {
        if(!pThis->mDataList.empty())
        {
            pthread_mutex_lock(&pThis->mMutex);
            string msg = pThis->mDataList.front();
            pThis->mDataList.pop_front();
            pthread_mutex_unlock(&pThis->mMutex);
            if(!pThis->mMsgHdlr) pThis->mMsgHdlr((void *)msg.c_str());
            pThis->OnRecvMessage(msg);
        }
        usleep(100);
    }
}


