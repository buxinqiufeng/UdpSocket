

#include "ClientList.h"




bool ClientList::isValid(ClientInfo& c)
{
	return true;
}

void ClientList::Add(ClientInfo& c)
{
	if(!isValid(c))
	{
		return;
	}
	Delete(c);
	mClientList.push_back(c);
	c.dump();
}

void ClientList::Delete(ClientInfo& c)
{
	list<ClientInfo>::iterator it;
	for(it=mClientList.begin(); it!=mClientList.end(); it++)
	{
		if(it->ip == c.ip && it->port == c.ip)
		{
			mClientList.remove(*it);
		}
	}
}

void ClientList::Empty()
{
	mClientList.empty();
}


bool ClientList::Find(string username, ClientInfo& c)
{
	list<ClientInfo>::iterator it;
	for(it=mClientList.begin(); it!=mClientList.end(); it++)
	{
		if(it->userName == username)
		{
			c = *it;
			return true;
		}
	}
	return false;
}

bool ClientList::Find(string ip, string port, ClientInfo& c)
{
	list<ClientInfo>::iterator it;
	for(it=mClientList.begin(); it!=mClientList.end(); it++)
	{
		if(it->ip == ip && it->port == port)
		{
			c = *it;
			return true;
		}
	}
	return false;
}

ClientInfo* ClientList::FindBySocket(int socket)
{
    list<ClientInfo>::iterator it;
	for(it=mClientList.begin(); it!=mClientList.end(); it++)
	{
		if(socket == it->socket)
		{
			return &(*it);
			//return true;
		}
	}
	return 0;
}

void ClientList::DeleteBySocket(int socket)
{
    list<ClientInfo>::iterator it;
	for(it=mClientList.begin(); it!=mClientList.end(); it++)
	{
		if(it->socket == socket)
		{
			mClientList.remove(*it);
		}
	}
}






