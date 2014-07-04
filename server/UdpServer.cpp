
#include <arpa/inet.h>
#include "UdpServer.h"
#include "stdio.h"

int UdpServer::mMaxListen = 30;
UdpServer::UdpServer()
{
    mServerSocket = 0;
    mOperaFunc = 0;
    mAcceptFunc = 0;
    mDisconnectFunc = 0;
    pthread_mutex_init(&mMutex, NULL);
    FD_ZERO(&mReadFd);
    mCliList.clear();
    mDataList.empty();
}
UdpServer::~UdpServer()
{
    Uninitialize();
}

void UdpServer::SetCustomHandler(unsigned long acceptFunc, unsigned long disconnectFunc)
{
    mAcceptFunc = (pAcceptHandler)acceptFunc;
    mDisconnectFunc = (pDisconnectHandler)disconnectFunc;
}

bool UdpServer::Initialize(const int port, unsigned long recvFunc)
{
    if(0 != recvFunc)
    {
        mOperaFunc = (pRecvMsgHandler)recvFunc;
    }
    Uninitialize();

    mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == mServerSocket)
    {
        cout << "socket init error." << endl;
        return false;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr,0, sizeof(struct sockaddr_in));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(port);

	if(-1 == bind(mServerSocket,(struct sockaddr *)(&serverAddr), sizeof(struct sockaddr)))
	{
		cout << "bind error." << endl;
		return false;
	}

	if(-1 == listen(mServerSocket,mMaxListen))
	{
		cout << "listen error." << endl;
		return false;
 	}

 	//创建线程接收socket连接
 	if(0 != pthread_create(&mAcceptThread, NULL, AcceptThread, this))
    {
        cout << "create accept thread failed" << endl;
        return false;
    }

    if(0 != pthread_create(&mManageThread, NULL, ManageThread, this))
    {
        cout << "create accept thread failed" << endl;
        return false;
    }

    return true;
}
bool UdpServer::SendData(const string &data, int socket)
{
    if(data.empty())
    {
        return false;
    }

    int res = send(socket, data.c_str(), data.length()+1, 0);
    if(-1 == res)
    {
        cout << "send error." << endl;
        return false;
    }
    return true;
}
bool UdpServer::Uninitialize()
{
    pthread_mutex_destroy(&mMutex);
    CloseAll();
    return true;
}

//接收socket连接线程
void * UdpServer::AcceptThread(void * pParam)
{
    if(0 == pParam)
    {
        cout << "param is null." << endl;
        return 0;
    }
    UdpServer * pThis = (UdpServer*)pParam;

    int maxFd = 0;
    struct sockaddr_in cliAddr;
    while(true)
    {
        FD_ZERO(&pThis->mReadFd);
        FD_SET(pThis->mServerSocket, &pThis->mReadFd);
        maxFd = (pThis->mServerSocket > maxFd)? pThis->mServerSocket : maxFd;

        for(list<int>::iterator itor = pThis->mCliList.begin(); itor != pThis->mCliList.end(); itor++)
        {
            FD_SET(*itor, &pThis->mReadFd);
            maxFd = (*itor > maxFd)? *itor : maxFd;
        }

        int res = select(maxFd + 1, &pThis->mReadFd, 0, 0, NULL);
        if(-1 == res)
        {
            cout << "select failed." << endl;
        }


        if(-1 != pThis->mServerSocket && FD_ISSET(pThis->mServerSocket , &pThis->mReadFd))
        {
            int sin_size = sizeof(struct sockaddr_in);
            int clientSoc = accept(pThis->mServerSocket, (struct sockaddr *)(&cliAddr), (socklen_t*)&sin_size);
            if(-1 == clientSoc)
            {
                cout << "accept error." << endl;
                continue;
            }

            pThis->mCliList.push_front(clientSoc);

            string ip = inet_ntoa(cliAddr.sin_addr);
            char tmp[16] = {0};
            sprintf(tmp, "%d", ntohs(cliAddr.sin_port));
            string port = tmp;
            if(0 != pThis->mAcceptFunc)
            {
                pThis->mAcceptFunc(ip, port, clientSoc);
            }
            pThis->OnClientConnect(ip, port, clientSoc);
        }

        for(list<int>::iterator itor = pThis->mCliList.begin(); itor != pThis->mCliList.end(); itor++)
        {
            if(-1 != *itor && FD_ISSET(*itor , &pThis->mReadFd))
            {
                char buf[MAX_MESSAGE_LEN] = {0};
                int res = recv(*itor, buf, sizeof(buf), 0);
                if(0 < res)
                {
                    pthread_mutex_lock(&pThis->mMutex);
                    pThis->mDataList.push_back(MsgInfo(*itor, buf));
                    pthread_mutex_unlock(&pThis->mMutex);
                }
                else if(0 == res)
                {
                    cout << "client quit." << endl;
                    pthread_mutex_lock(&pThis->mMutex);
                    pThis->mCliList.remove(*itor);
                    itor--;
                    pThis->mDisconnectFunc(*itor);
                    pThis->OnClientDisconnect(*itor);
                    pthread_mutex_unlock(&pThis->mMutex);
                }
                else
                {
                    cout << "receive failed." << endl;
                }
            }
        }
    }
}

//管理线程，用于创建处理线程
void * UdpServer::ManageThread(void * pParam)
{
    if(!pParam)
    {
        return 0;
    }
    pthread_t pid;
    UdpServer * pThis = (UdpServer *)pParam;
    while(1)
    {
        //使用互斥量
        pthread_mutex_lock(&pThis->mMutex);
        int nCount = pThis->mDataList.size();
        pthread_mutex_unlock(&pThis->mMutex);
        if(nCount > 0)
        {
            pid = 0;
            //创建处理线程
            if( 0 != pthread_create(&pid, NULL, OperatorThread, pParam))
            {
                cout << "creae operator thread failed\n" << endl;
            }
        }
        //防止抢占CPU资源
        usleep(100);
    }
}

void * UdpServer::OperatorThread(void * pParam)
{
    if(!pParam)
    {
        return 0;
    }

    UdpServer * pThis = (UdpServer*)pParam;
    pthread_mutex_lock(&pThis->mMutex);
    MsgInfo data;
    if(pThis->mDataList.size() > 0)
    {
        data = pThis->mDataList.front();
        pThis->mDataList.pop_front();
        if(pThis->mOperaFunc)
        {
            pThis->mOperaFunc(data.Content, data.Fd, data.Time);
        }
        pThis->OnReceiveMessage(data.Content, data.Fd, data.Time);
    }
    pthread_mutex_unlock(&pThis->mMutex);
    return 0;
}

void UdpServer::CloseSocket(int socket)
{
    close(socket);
    for(list<int>::iterator itor = mCliList.begin(); itor != mCliList.end(); itor++)
    {
        if(*itor == socket)
        {
            mCliList.remove(*itor);
            break;
        }
    }
}

void UdpServer::CloseAll()
{
    for(list<int>::iterator itor = mCliList.begin(); itor != mCliList.end(); itor++)
    {
        close(*itor);
    }
    mCliList.empty();
}


