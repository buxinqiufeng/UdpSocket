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
    AddClient(ClientInfo(ip, port, socket, CIS_Connected));
}

void ServerCenter::OnReceiveMessage(const string msg, const int socket, const int time)
{
    Message message;
    if(!message.ParaseMessage(msg.c_str()))
    {
        cout << "Message parase failed:" << msg << endl;
    }
    const string type = message.GetMessageType();
    cout << "Message type=(" << type << "), login type is(" << MessageTag::TypeLogin << ")\n";
    if(type == MessageTag::TypeLogin)
    {
        HandleLoginMsg(message, socket);
    }
    else if(MessageTag::TypeSync == type)
    {

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
    bool verifySucess = false;
    ClientInfo *client = GetClientByFd(fd);
    if(null == client || CIS_Connected != client->State)
    {
        cout << "Client state invalid, discard message." << endl;
        return false;
    }

    string username = msg.GetValue(MessageTag::UserName);
    string password = msg.GetValue(MessageTag::Password);
    if(!VerifyUserLogin(username, password))
    {
        verifySucess = false;
        cout << client->UserName << " Login failed for wrong password." << endl;
    }
    else
    {
        verifySucess = true;
        client->UserName = username;
        client->Password = password;
        client->State = CIS_Logined;
        cout << "New user Login:" << endl << client->dump() << endl;
    }

    //login response.
    Message message;
    string strErr;
    verifySucess ? strErr="0" : strErr = "-1";
    message.SetMessageType(MessageTag::TypeLogin);
    message.SetValue(MessageTag::ErrorCode, strErr);
    if(!SendData(message.GenerateMessage(), fd))
    {
        return false;
    }
    return true;
}

bool ServerCenter::HandleSyncMsg(Message& msg, int fd)
{
    list<ClientInfo>::iterator it;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		if(it->Fd == fd)
            continue;

        Message msg;
        msg.SetMessageType(MessageTag::TypeSync);
        msg.SetValue(MessageTag::UserName, it->UserName);
        msg.SetValue(MessageTag::Ip, it->Ip);
        msg.SetValue(MessageTag::Port, it->Port);
        msg.SetValue(MessageTag::Status, ConventStateToString(it->State));
        SendData(msg.GenerateMessage(), fd);
	}

	//send sync end message.
	{
        Message msg;
        msg.SetMessageType(MessageTag::TypeSyncEnd);
        SendData(msg.GenerateMessage(), fd);
	}
	return true;
}

string ServerCenter::ConventStateToString(ClientInfoStatus cis)
{
    string strState;
    switch(cis)
    {
     case CIS_Invalid:
        strState = "0";
        break;
    case CIS_Connected:
        strState = "1";
    case CIS_Logined:
        strState = "2";
    case CIS_Logout:
        strState = "3";
    default:
        strState = "-1";
    }
    return strState;
}

bool ServerCenter::HandleChatMsg(Message& msg, int fd)
{
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

void ServerCenter::ClientLoginNotify(ClientInfo &ci)
{
    list<ClientInfo>::iterator it;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		if(it->Fd == ci.Fd)
            continue;

		Message msg;
        msg.SetMessageType(MessageTag::TypeSync);
        msg.SetValue(MessageTag::UserName, it->UserName);
        msg.SetValue(MessageTag::Ip, it->Ip);
        msg.SetValue(MessageTag::Port, it->Port);
        msg.SetValue(MessageTag::Status, ConventStateToString(it->State));
        SendData(msg.GenerateMessage(), ci.Fd);
	}
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

void ServerCenter::Dump()
{
    cout << "*******************************************" << endl;
    cout << "Server is running. there are " << mClist.size() << "client online." << endl;
    list<ClientInfo>::iterator it;
    int i=0;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		cout << i++ << " . " << it->dump() << endl;
	}
    cout << "*******************************************" << endl;
}
