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
        HandleLoginMsg(message, socket);
    }
    else if(MessageTag::TypeChat == type)
    {
        HandleChatMsg(message, socket);
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

bool ServerCenter::HandleLoginMsg(Message& msg, int fd)
{
    if(MessageTag::TypeLogin == msg.GetMessageType())
    {
        return false;
    }
    ClientInfo *client = GetClientByFd(fd);
    if(null != client || Connected != client->State)
    {
        cout << "Invalid message received, discard it." << endl;
        return false;
    }

    string username = msg.GetValue(MessageTag::UserName);
    string password = msg.GetValue(MessageTag::Password);
    if(!VerifyUserLogin(username, password))
    {
        cout << client->UserName << " Login failed for wrong password." << endl;
    }

    client->UserName = username;
    client->Password = password;
    client->State = Logined;
    cout << "New user Login:" << endl << client->dump() << endl;

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

bool ServerCenter::HandleChatMsg(Message& msg, int fd)
{
    if(MessageTag::TypeChat == msg.GetMessageType())
    {
        return false;
    }
    ClientInfo *to = GetClientByAddress(msg.GetValue(MessageTag::Ip), msg.GetValue(MessageTag::Port));
    ClientInfo *from = GetClientByFd(fd);
    if(null == to)
    {
        cerr << "Message from " << from->UserName << " to unknown user, discard it." << endl;
    }

    //send message to enduser.
    Message newMsg = msg;
    newMsg.SetValue(MessageTag::Ip, from->Ip);
    newMsg.SetValue(MessageTag::Port, from->Port);
    if(!SendData(newMsg.GenerateMessage(), to->Fd))
    {
        return false;
    }
    return true;
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

ServerCenter::ClientInfo* ServerCenter::GetClientByAddress(const string ip, const string port)
{
    list<ClientInfo>::iterator it;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		if(it->Ip == ip && it->Port == port)
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

void SetOutStream(ostream os)
{

}
