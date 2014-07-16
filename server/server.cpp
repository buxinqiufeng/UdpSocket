
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
using namespace std;

#include "ClientList.h"
#include "ClientInfo.hpp"
#include "../common/Common.h"
#include "UdpServer.h"
#include "../common/Message/Message.h"
#include "ServerCenter.h"

unsigned short port_num = 0x8888;
#define max_cmd_len 2048
/*
ClientList gClist;

bool parseCmd(char *cmd, ClientInfo& c, int &mode);

void RecvHandle(const char * szBuf, size_t nLen, int socket, UdpServer &server);
void AcceptHandle(string ip, string port, int handle);
void DisconnectHandle(int socket);
bool HandleLoginMsg(const char* msg, int socket, UdpServer &server);
bool HandleGenericMsg(const char* msg, int socket, UdpServer &server);
bool LoginVerfy(string username, string password);
int main()
{
    UdpServer server;
    server.Initialize(port_num, (unsigned long)RecvHandle);
    server.SetCustomHandler((unsigned long)AcceptHandle, (unsigned long)DisconnectHandle);
    while(1)
    {
        sleep(100);
    }
    server.Uninitialize();
    return 0;
}

void RecvHandle(const char * szBuf, size_t nLen, int socket, UdpServer &server)
{
    ClientInfo* client;
    if(0 != (client = gClist.FindBySocket(socket)))
    {
        if(DEBUG)
            cout << "Message(" << client->ip << ":" << client->port << "):" << szBuf << endl;
        switch(client->status)
        {
        case ClientInfo::Login:
            HandleLoginMsg(szBuf, socket, server);
            break;
        case ClientInfo::Connected:
            HandleGenericMsg(szBuf, socket, server);
        default :
            cout << "Unknown client status:" << client->status;
            break;
        }
    }
    else
    {
        cout << "Unknown message received, discard it." << endl;
    }
}

void AcceptHandle(string ip, string port, int handle)
{
    ClientInfo cl;
    cl.ip = ip;
    cl.port = port;
    cl.socket = handle;
    cl.status = ClientInfo::Login;
    gClist.Add(cl);
}

void DisconnectHandle(int socket)
{
    gClist.DeleteBySocket(socket);
}

bool HandleLoginMsg(const char* msg, int socket, UdpServer &server)
{
    string errCode = "0";
    //login verify.
    ClientInfo* client;
    if(0 == (client =gClist.FindBySocket(socket)))
    {
        cout << "HandleLoginMsg falied." << endl;
        return false;
    }
    client->dump();

    //Login
    {
        Message message;
        message.ParaseMessage(msg);
        string mode = message.GetValue(MessageTag::Mode);
        client->userName = message.GetValue(MessageTag::UserName);
        client->password = message.GetValue(MessageTag::Password);
        if(LoginVerfy(client->userName, client->password))
        {
            errCode = "1";
            client->status = ClientInfo::Connected;
        }
        else
        {
            errCode = "-1";
        }
    }


    //login response.
    {
        Message message;
        message.SetMessageType(MessageTag::TypeLogin);
        message.SetValue(MessageTag::ErrorCode, errCode);
        if(server.SendData(message.GenerateMessage(), socket))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool HandleGenericMsg(const char* msg, int socket, UdpServer &server)
{
    const string typeChat = "Chat";
    TiXmlDocument *txdCmd = new TiXmlDocument();
    txdCmd->Parse((char *)msg);
    TiXmlElement *RootElement = txdCmd->RootElement();
    const string msgType = RootElement->Attribute("Type");
    if(typeChat == msgType)
    {
        string content = RootElement->Attribute("Content");
        ClientInfo *cl = gClist.FindBySocket(socket);
        if(cl)
        {
            cout << "Receive message from(" << cl->ip << ":" << cl->port << "):" << content << endl;
        }
        else
        {
            cout << "Message from unknown client, discard it." << endl;
            server.CloseSocket(socket);
            return false;
        }

        Message msg;
        msg.SetMessageType(MessageTag::TypeChat);
        msg.SetValue(MessageTag::Content, "Message received, hehe");
        if(server.SendData(msg.GenerateMessage(), socket))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
bool LoginVerfy(string username, string password)
{
    string user = "username";
    string pwd = "password";
    if(user == username && password == pwd)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int main_bak()
{
	int sfp,nfp;
	struct sockaddr_in s_add,c_add;
	int sin_size;
	char buffer[max_cmd_len]={0};
	int recbytes;

	if(DEBUG) printf("Hello,welcome to my server !\r\n");
	sfp = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sfp)
	{
		printf("socket fail ! \r\n");
		return -1;
	}
	if(DEBUG) printf("socket ok !\r\n");


	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr=htonl(INADDR_ANY);
	s_add.sin_port=htons(port_num);

	if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("bind fail !\r\n");
		return -1;
	}
	if(DEBUG) printf("bind ok !\r\n");

	if(-1 == listen(sfp,5))
	{
		printf("listen fail !\r\n");
		return -1;
	}
	if(DEBUG) printf("listen ok\r\n");

	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);

		nfp = accept(sfp, (struct sockaddr *)(&c_add), (socklen_t*)&sin_size);
		if(-1 == nfp)
		{
			printf("accept fail !\r\n");
			return -1;
		}
		if(DEBUG) printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));

		memset(buffer, 0, max_cmd_len);
		if(-1 == (recbytes = read(nfp,buffer,max_cmd_len)))
		{
			printf("read data fail !\r\n");
			return -1;
		}

		ClientInfo cl;
		int mode=-1;
        {
            TiXmlDocument *txdCmd = new TiXmlDocument();
            txdCmd->Parse(buffer);
            TiXmlElement *RootElement = txdCmd->RootElement();
            if(TIXML_SUCCESS != RootElement->QueryIntAttribute("Mode", &mode))
            {
                cout << "mode read failed." << endl;
            }
            cl.userName = RootElement->Attribute("UserName");
            cl.password = RootElement->Attribute("Password");
        }
		cl.ip = inet_ntoa(c_add.sin_addr);
		char tmp[16] = {0};
		sprintf(tmp, "%d", ntohs(c_add.sin_port));
		cl.port = tmp;
		if(DEBUG) cout << mode<<endl;

		if(0 == mode)
		{
			gClist.Add(cl);
		}
		else if(1 == mode)
		{
            ClientInfo cl2;
		    gClist.Find(cl.userName, cl2);

            {
                TiXmlDocument *txdCmd = new TiXmlDocument();
                TiXmlElement *RootElement = new TiXmlElement("Message");
                txdCmd->LinkEndChild(RootElement);
                RootElement->SetAttribute("Type", "1");
                RootElement->SetAttribute("ErrorCode", "0");
                RootElement->SetAttribute("Ip", cl2.ip.c_str());
                RootElement->SetAttribute("Port", cl2.port.c_str());
                TiXmlPrinter printer;
                txdCmd->Accept(&printer);
                const char *cmd = printer.CStr();
                cout << cmd << endl;

                if(-1 == write(nfp,cmd,strlen(cmd)+1))
                {
                    printf("write fail!\r\n");
                    continue;;
                }
            }
		}
		else
		{
			cout << "error mode." << endl;
		}
		close(nfp);

	}
	close(sfp);
	return 0;
}

bool parseCmd(char *cmd, ClientInfo& c, int &mode)
{
	if(DEBUG) cout << "command: " << cmd << endl;
	int len = strlen(cmd);
	if(max_cmd_len < len)
	{
		cout << "command len invalid!" << endl;
		return false;
	}

	char *username = cmd;
	char *gap = strchr(cmd, ' ');
	if(NULL == gap)
	{
		cout << "username parse failed!" << endl;
		return false;
	}
	gap[0] = '\0';

	char *password = gap+1;
	gap = strchr(gap+1, ' ');
	if(NULL == gap)
	{
		cout << "mode parse failed!" << endl;
		return false;
	}
	gap[0] = '\0';
	c.id = username;
	c.password = password;

	mode = gap[1]-'0';

	if(DEBUG) cout << "new client: " << c.id << " " << c.password << endl;
	return true;
}

*/

int main()
{
    ServerCenter sc(port_num);
    sc.Start();
    while(1)
    {
        sleep(1000);
    }
}










