#include "ServerCenter.h"
#include "../common/Message/Message.h"

ServerCenter::ServerCenter()
{
    Initialize();
}

ServerCenter::ServerCenter(int port)
{
    Initialize();
    SetPort(port);
}

void ServerCenter::Initialize()
{
    mPort = 0;
    mMaxMsgLen = 0;
    mClist.empty();
}

void ServerCenter::Start()
{
    if(0 == mPort)
    {
        cout << "Please set port first." << endl;
    }
    UdpServer::Initialize(mPort, 0);
}

void ServerCenter::Stop()
{
    Uninitialize();
    Initialize();
}

void ServerCenter::OnClientConnect(string ip, string port, int socket)
{
    AddClient(ClientInfo(ip, port, socket));
}

void ServerCenter::OnReceiveMessage(const string msg, const int socket, const int time)
{
    Message message;
    if(!message.ParaseMessage(msg.c_str()))
    {
        cout << "Message parase failed:" << msg << endl;
    }
    const string type = message.GetMessageType();

    if(MessageTag::TypeLogin == type)
    {
        HandleLoginMsg(msg, socket);
    }
    else if(MessageTag::TypeChat == type)
    {
        HandleChatMsg(msg, socket);
    }
    else if(MessageTag::TypeLogout == type)
    {

    }
    else
    {
        cout << "Invalid message received, discard it." << endl;
    }
}

void ServerCenter::OnClientDisconnect(int socket)
{
    DeleteClientByFd(socket);
}

bool ServerCenter::HandleLoginMsg(const string msg, int fd)
{
    ClientInfo *client = GetClientByFd(fd);
    if(null != client || Connected != client->State)
    {
        cout << "Invalid message received, discard it." << endl;
        return false;
    }

    if(!VerifyUserLogin(client->UserName, client->Password))
    {
        cout << client->UserName << " Login failed for wrong password." << endl;
    }

    client->State = Logined;

    //login response.
    Message message;
    message.SetMessageType(MessageTag::TypeLogin);
    message.SetValue(MessageTag::ErrorCode, "0");
    if(!SendData(message.GenerateMessage(), fd))
    {
        return false;
    }
    return true;
}

bool ServerCenter::HandleChatMsg(const string msg, int fd)
{
    return false;
}

void ServerCenter::AddClient(const ClientInfo& client)
{
    if(null != GetClientByFd(client.Fd))
    {
        DeleteClientByFd(client.Fd);
    }
    mClist.push_back(client);
}

ServerCenter::ClientInfo* ServerCenter::GetClientByFd(const int fd)
{
    list<ClientInfo>::iterator it;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		if(it->Fd == fd)
		{
			return &(*it);
		}
	}
	return null;
}

void ServerCenter::DeleteClientByFd(const int fd)
{
    list<ClientInfo>::iterator it;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		if(it->Fd == fd)
		{
			mClist.remove(*it);
		}
	}
}

bool ServerCenter::VerifyUserLogin(string name, string pw)
{
    return true;
}
