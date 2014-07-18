#ifndef __CLIENTCENTER_H__
#define __CLIENTCENTER_H__
#include <iostream>
#include <list>
using namespace std;

#include "../common/Common.h"
#include "UdpClient.h"
#include "../common/Message/Message.h"

class ClientCenter:public UdpClient
{
public:
    //status of friends.
    enum ClientInfoStatus
    {
        CIS_Invalid = 0,
        CIS_Connected,
        CIS_Logined,
        CIS_Logout
    };

    //status of client.
    enum ClientStatus
    {
        CS_Invalid = 0,
        CS_Connected,
        CS_Syncing,
        CS_Logined,
        CS_Logout
    };

    struct ClientInfo
    {
        ClientInfo():State(CIS_Invalid){}
        bool operator==(const ClientInfo &client)
        {
            if(client.Fd == Fd
               && client.Ip == Ip
               && client.Port == Port
               && client.UserName == UserName
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
        ClientInfoStatus State;
    };
public:
    ClientCenter();
    bool Start(const string ip, const int port);
    void Login(const string username, const string password);
    void Stop();
    void Dump();

protected:
    virtual void OnConnected();
    virtual void OnDisconnect();
    virtual void OnRecvMessage(const string msg);

private:
    void HandleLoginMessage(Message &msg);
    void SendSyncRequest();
    void HandleSyncMsg(Message& msg);

    void AddUser(const ClientInfo& cl);
    ClientInfo* FindUserByName(const string name);
    string GetErrorMessage(string errcode);
    ClientInfoStatus ConventToLocalStatus(string status);


private:
    list<ClientInfo> mClist;
    ClientStatus mStatus;
    string mSerIp;
    int mSerPort;
    string mUsername;
};

#endif
