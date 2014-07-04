#ifndef __CLIENTLIST_H__
#define __CLIENTLIST_H__

#include <iostream>
#include <string>
#include <list>
#include <memory>
using namespace std;

#include "ClientInfo.hpp"

class ClientList
{
public:
public:
	void Add(ClientInfo& c);
	void Delete(ClientInfo& c);
	void Empty();
	bool Find(string username, ClientInfo& c);
	bool Find(string ip, string port, ClientInfo& c);
	ClientInfo* FindBySocket(int socket);
	void DeleteBySocket(int socket);
    list<ClientInfo>::iterator GetIteratorBegin(){return mClientList.begin();}
    list<ClientInfo>::iterator GetIteratorEnd(){return mClientList.end();}

private:
	bool isValid(ClientInfo& c);


	list<ClientInfo> mClientList;
};

#endif
