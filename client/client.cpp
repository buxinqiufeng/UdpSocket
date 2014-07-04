
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <list>
using namespace std;

#include "../common/tinyxml/tinyxml.h"
#include "../common/Common.h"
#include "../common/MessageTag.h"

const char default_server_addr[]="127.0.0.1";
const unsigned short port_num = 0x8888;

list<string> gDataList;
pthread_mutex_t gMutex;

bool ParseArgs(int argc, char **argv, char *server, char *username, char *password, int &mode);
bool Login(int serverSocket);
bool Sync(int serverSocket);
void* RecvThread(void *param);
void* HandleDataThread(void *param);
int main(int argc, char **argv)
{
	int cfd;
	int recbytes;
	//int sin_size;
	char buffer[1024]={0};

	char server[16] = {0};
	char username[32] = {0};
	char password[32] = {0};
	int mode = -1;

    gDataList.empty();

	if(!ParseArgs(argc, argv, server, username, password, mode))
	{
		cout << "error parse args" << endl;
		return -1;
	}
 	if(DEBUG) cout<<"server:"<<server<<",username:"<<username<<",password:"<<password<<",mode:"<<mode<<endl;

	struct sockaddr_in s_add;

	printf("Hello,welcome to client !\r\n");

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == cfd)
	{
		printf("socket fail ! \r\n");
		return -1;
	}
	printf("socket ok !\r\n");

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	if(0 != strlen(server))
	{
		s_add.sin_addr.s_addr= inet_addr(server);
	}
	else
	{
		s_add.sin_addr.s_addr= inet_addr(default_server_addr);
	}
	s_add.sin_port=htons(port_num);
	printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);


    //connect.
	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("connect fail !\r\n");
		return -1;
	}
	printf("connect ok !\r\n");

    while(1)
    {

    }


    pthread_t mRecvThread = 0;
    if(0 != pthread_create(&mRecvThread, NULL, RecvThread, &cfd))
    {
        cout << "RecvThread create failed." << endl;
    }


    //chat
    while(1)
    {
        //send.
        {
            string msg;
            cin >> msg;
            TiXmlDocument *txdCmd = new TiXmlDocument();
            TiXmlElement *RootElement = new TiXmlElement("Message");
            txdCmd->LinkEndChild(RootElement);
            RootElement->SetAttribute(MessageTag::Type, MessageTag::TypeChat);
            RootElement->SetAttribute(MessageTag::Content, msg);
            TiXmlPrinter printer;
            txdCmd->Accept(&printer);
            const char *cmd = printer.CStr();
            if(-1 == write(cfd,cmd,strlen(cmd)+1))
            {
                cout << "send message failed." << endl;
                goto end;
            }
        }

        //receive
        {
            memset(buffer, 0, 1024);
            if(-1 == (recbytes = read(cfd,buffer,1024)))
            {
                printf("read data fail !\r\n");
                goto end;
            }
            TiXmlDocument *txdCmd = new TiXmlDocument();
            txdCmd->Parse(buffer);
            TiXmlElement *RootElement = txdCmd->RootElement();
            const string msgType = RootElement->Attribute(MessageTag::Type);
            const string typeChat = MessageTag::TypeChat;
            if(typeChat == msgType)
            {
                string content = RootElement->Attribute(MessageTag::Content);
                cout << content << endl;
            }
            else
            {
                cout << "error message type when receive chat:type=" << msgType << endl;
            }
        }
    }

end:
	close(cfd);
	return 0;
}

bool Login(int serverSocket)
{
    {//Login.
        TiXmlDocument *txdCmd = new TiXmlDocument();
        TiXmlElement *RootElement = new TiXmlElement(MessageTag::Message);
        txdCmd->LinkEndChild(RootElement);
        RootElement->SetAttribute(MessageTag::Type, "1");
        RootElement->SetAttribute(MessageTag::ErrorCode, "0");
        RootElement->SetAttribute(MessageTag::Mode, "0");
        RootElement->SetAttribute(MessageTag::UserName, "username");
        RootElement->SetAttribute(MessageTag::Password, "password");
        TiXmlPrinter printer;
        txdCmd->Accept(&printer);
        const char *cmd = printer.CStr();
        if(DEBUG)cout << cmd << endl;
        if(strlen(cmd) > MAX_MESSAGE_LEN - 1)
        {
            cout << "Warning:message length reached max message len." << endl;
        }
        if(-1 == write(serverSocket,cmd,strlen(cmd)+1))
        {
            printf("write fail!\r\n");
            return false;
        }
    }

    {//wait for login response.
	    char buffer[MAX_MESSAGE_LEN]={0};
	    if(-1 == (read(serverSocket,buffer,MAX_MESSAGE_LEN)))
		{
			printf("read data fail !\r\n");
			return false;
		}
		TiXmlDocument *txdCmd = new TiXmlDocument();
        txdCmd->Parse(buffer);
        TiXmlElement *RootElement = txdCmd->RootElement();
        string msgType = RootElement->Attribute(MessageTag::Type);
        string errCode = RootElement->Attribute(MessageTag::ErrorCode);
        string LoginSuccess = MessageTag::TypeLogin;
        string errOne = "1";
        if(msgType == LoginSuccess && errCode == errOne)
        {
            //login success.
            if(DEBUG) cout << "Login success" << endl;
        }
        else
        {
            cout << "login failed, msgtype=" << msgType << ", errcode=" << errCode << endl;
            return false;
        }
	}

	return true;
}

bool Sync(int serverSocket)
{
    return false;
}

void* RecvThread(void *param)
{
    if(0 == param)
    {
        return 0;
    }

    int serSocket = *((int*)(param));

    while(1)
    {
        char buffer[MAX_MESSAGE_LEN]={0};
	    if(-1 == (read(serSocket,buffer,MAX_MESSAGE_LEN)))
		{
			printf("read data fail !\r\n");
			return 0;
		}
		else
		{
		    string msg = buffer;
		    pthread_mutex_lock(&gMutex);
		    gDataList.push_back(msg);
		    pthread_mutex_unlock(&gMutex);
		}
    }

    return 0;
}

void* HandleDataThread(void *param)
{
    while(1)
    {
        if(gDataList.size() > 0)
        {
            //string msg = gDataList.pop_back();
        }
    }
    return 0;
}

bool ParseArgs(int argc, char **argv, char *server, char *username, char *password, int &mode)
{
	if(NULL == server || NULL == username || NULL == password)
	{
		cout << "input value error." << endl;
		return false;
	}
	for(int i=1; i<argc; i++)
	{
		if(0 == strcmp("-m", argv[i]))
		{
			i++;
			mode = argv[i][0]-'0';
			continue;
		}
		else if(0 == strcmp("-s", argv[i]))
		{
			i++;
			strcpy(server, argv[i]);
			continue;
		}

		strcpy(username, argv[i]);
		if(i<argc-1)
		{
			strcpy(password, argv[i+1]);
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

void Usage()
{
	cout << "-m mode [-s server] username password" << endl
		<< "-m : connect mode, 0 login, 1 connect to other client." << endl
		<< "-s : server address, if not set, use default server."  << endl;
}




