#ifndef __SERVERCENTER_H__
#define __SERVERCENTER_H__
#include <iostream>
#include <list>
using namespace std;
#include <time.h>
#include "UdpServer.h"
#include "../common/Message/Message.h"

class ServerCenter:public UdpServer
{
private:
    enum ClientInfoStatus
    {
        Invalid = 0,
        Connected,
        Logined,
        Logout
    };
    struct ClientInfo
    {
        ClientInfo():State(Invalid){}
        ClientInfo(string ip, string port, int socket):Fd(socket),Ip(ip),Port(port),State(Invalid){}
        bool operator==(const ClientInfo &client)
        {
            if(client.Fd == Fd
               && client.Ip == Ip
               && client.Port == Port
               && client.UserName == UserName
               && client.Password == Password
               && client.State == State)
               {
                   return true;
               }
               return false;
        }
        string dump()
        {
            string info = UserName;
            info += "(";
            info += Ip;
            info += ":";
            info += Port;
            info +=")";
            info +=" State=";
            info +=State;
            return info;
        }
        int Fd;
        string Ip;
        string Port;
        string UserName;
        string Password;
        ClientInfoStatus State;
    };

public:
    ServerCenter();
    ServerCenter(int port);
    void SetPort(int port){mPort = port;};
    void Start();
    void Stop();
    void SetMaxMessageLen(int len){mMaxMsgLen = len;}

private:
    void AddClient(const ClientInfo& client);
    ClientInfo* GetClientByFd(const int fd);
    ClientInfo* GetClientByAddress(const string ip, const string port);
    void DeleteClientByFd(const int fd);

    bool HandleLoginMsg(Message& msg, int fd);
    bool VerifyUserLogin(string name, string pw);
    bool HandleChatMsg(Message& msg, int fd);

    virtual void OnClientConnect(string ip, string port, int socket);
    virtual void OnReceiveMessage(const string msg, const int socket, const int time);
    virtual void OnClientDisconnect(int socket);
    void Initialize();
private:
    int mPort;
    int mMaxMsgLen;
    list<ClientInfo> mClist;
};

#endif
