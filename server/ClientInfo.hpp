#ifndef __CLIENT_INFO_HPP__
#define __CLIENT_INFO_HPP__

struct ClientInfo
{
	enum Status
	{
		Init=0,
		Login,
		Sync,
		Connected,
		Stop
	};
	string id;
	string password;
	string ip;
	string port;
	string userName;
	string loginPassword;
	Status status;
	int socket;


    ClientInfo()
    {
        status = Init;
    }
    ClientInfo& operator=(const ClientInfo& c)
	{
        if(this != &c)
        {
            id = c.id;
            password = c.password;
            ip = c.ip;
            port = c.port;
            userName = c.userName;
            loginPassword = c.loginPassword;
            status = c.status;
            socket = c.socket;
        }
	return *this;
	}
	bool operator==(const ClientInfo& c)
	{
        if(id == c.id
            && password == c.password
            && ip == c.ip
            && port == c.port
            && userName == c.userName
            && loginPassword == c.loginPassword
            && status == c.status
            && socket == c.socket)
        {
            return true;
        }
        return false;
    }
	void dump()
	{
        cout << "Client Information: " << userName << " " << password << " " << ip << " " << port << " status=" << status<< endl;
    }
};
#endif
