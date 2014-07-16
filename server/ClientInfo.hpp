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
	string ip;
	string port;
	string userName;
	Status status;


    ClientInfo()
    {
        status = Init;
    }
    ClientInfo& operator=(const ClientInfo& c)
	{
        if(this != &c)
        {
            id = c.id;
            ip = c.ip;
            port = c.port;
            userName = c.userName;
            status = c.status;
        }
	return *this;
	}
	bool operator==(const ClientInfo& c)
	{
        if(id == c.id
            && ip == c.ip
            && port == c.port
            && userName == c.userName
            && status == c.status)
        {
            return true;
        }
        return false;
    }
	string dump()
	{
        string info;
        info = userName;
        info += "(";
        info += ip;
        info += ":";
        info += port;
        info += ") status=";
        info += status;
        return info;
    }
};
#endif
