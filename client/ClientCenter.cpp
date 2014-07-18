

#include "ClientCenter.h"

ClientCenter::ClientCenter()
{
    mClist.empty();
    mStatus = CS_Invalid;
}

bool ClientCenter::Start(const string ip, const int port)
{
    bool res = Initialize(ip, port);
    if(res)
    {
        mSerIp = ip;
        mSerPort = port;
        return true;
    }
    else
    {
        return false;
    }
}

void ClientCenter::Stop()
{
    Disconnect();
    mStatus = CS_Logout;
}

void ClientCenter::OnConnected()
{
    mStatus = CS_Connected;
}

void ClientCenter::OnDisconnect()
{
    Disconnect();
    mStatus = CS_Logout;
}

void ClientCenter::Login(const string username, const string password)
{
    if(CS_Connected != mStatus)
    {
        cerr << "client does not connected." << endl;
        return;
    }

    Message msg;
    msg.SetMessageType(MessageTag::TypeLogin);
    msg.SetValue(MessageTag::UserName, username);
    msg.SetValue(MessageTag::Password, password);
    if(!SendData(msg.GenerateMessage()))
    {
        cerr << "Login failed." << endl;
        Disconnect();
        return;
    }

    mUsername = username;
}

string ClientCenter::GetErrorMessage(string errcode)
{
    stringstream ss;
    int iErr = 0;
    string strErr;
    ss << errcode;
    ss >> iErr;
    switch(iErr)
    {
    case 0:
        strErr = "Sucess.";
        break;
    case -1:
        strErr = "Verify failed.";
        break;
    default:
        strErr = "Unknown error.";
    }
    return strErr;
}

ClientCenter::ClientInfoStatus ClientCenter::ConventToLocalStatus(string status)
{
    stringstream ss;
    int iStatus;
    ss << status;
    ss >> iStatus;
    return (ClientCenter::ClientInfoStatus)iStatus;
}

void ClientCenter::HandleLoginMessage(Message &msg)
{
    if(CS_Connected != mStatus)
    {
        cerr << "Client does not Connected." << endl;
        return;
    }

    const string errcode = msg.GetValue(MessageTag::ErrorCode);
    const string errSucess = "0";
    if(errSucess != errcode)
    {
        cerr << GetErrorMessage(errcode) << endl;
    }
    else
    {
        SendSyncRequest();
        mStatus = CS_Syncing;
    }
}

void ClientCenter::SendSyncRequest()
{
    Message msg;
    msg.SetMessageType(MessageTag::TypeSync);
    if(!SendData(msg.GenerateMessage()))
    {
        cerr << "Sync failed." << endl;
        Disconnect();
    }
}

void ClientCenter::HandleSyncMsg(Message& msg)
{
    if(CS_Syncing != mStatus)
    {
        cerr << "Client does not prepare for sync." << endl;
        return;
    }
    ClientInfo cl;
    cl.UserName = msg.GetValue(MessageTag::UserName);
    cl.Ip = msg.GetValue(MessageTag::Ip);
    cl.Port = msg.GetValue(MessageTag::Port);
    cl.State = ConventToLocalStatus(msg.GetValue(MessageTag::Status));
    AddUser(cl);
}

ClientCenter::ClientInfo* ClientCenter::FindUserByName(const string name)
{
    list<ClientInfo>::iterator it;
	for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		if(it->UserName == name)
		{
			return &(*it);
		}
	}
	return null;
}

void ClientCenter::AddUser(const ClientInfo& cl)
{
    ClientInfo* oldCl = FindUserByName(cl.UserName);
    if(null != oldCl)
    {
        mClist.remove(*oldCl);
    }

    mClist.push_back(cl);
}

void ClientCenter::OnRecvMessage(const string msg)
{
    Message message;
    if(!message.ParaseMessage(msg.c_str()))
    {
        cerr << "Message parase failed:" << msg << endl;
    }
    const string type = message.GetMessageType();

    if(MessageTag::TypeLogin == type)
    {
        HandleLoginMessage(message);
    }
    else if(MessageTag::TypeSync == type)
    {
        HandleSyncMsg(message);
    }
    else if(MessageTag::TypeSyncEnd == type)
    {
        mStatus = CS_Logined;
    }
    else
    {
        cerr << "Unknown message received." << endl;
    }
}

void ClientCenter::Dump()
{
    cout << "*******************************************" << endl;
    cout << mUsername << " Connected to Server(" << mSerIp << ")" << endl;
    list<ClientInfo>::iterator it;
    int i = 0;
    for(it=mClist.begin(); it!=mClist.end(); it++)
	{
		cout << i++ << " . " << it->dump() << endl;
	}
    cout << "*******************************************" << endl;
}


