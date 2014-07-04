#ifndef __MESSAGEBASE_H__
#define __MESSAGEBASE_H__
#include <string>
using namespace std;

#include "../tinyxml/tinyxml.h"
#include "../Common.h"

class Message
{
public:
    static string INVALID_VALUE;
    static string ROOT_ELEMENT;
    Message()
    {
        mTag = "";
        mXmlDoc = new TiXmlDocument();
        isElementNew = false;
    }
    Message(const char* type):mTag(type)
    {
        mXmlDoc = new TiXmlDocument();
        isElementNew = false;
    }
    ~Message();
    void SetMessageType(const char* type);
    void SetMessageType(const string type);
    string GetMessageType();
    void SetValue(const char* attr, const char* value);
    void SetValue(const string attr, const char* value){SetValue(attr.c_str(), value);}
    void SetValue(const string attr, const string value){SetValue(attr.c_str(), value.c_str());}
    string GetValue(const char* attr);
    string& GenerateMessage();
    bool ParaseMessage(const char* msg);

private:
    string mTag;
    TiXmlDocument *mXmlDoc;
    TiXmlElement *mRootElement;
    bool isElementNew;
    string mStrMessage;
};


namespace MessageTag{
    const char * const Message = "Message";
    const char * const UserName = "UserName";
    const char * const Type = "Type";
    const char * const Mode = "Mode";
    const char * const Password = "Password";
    const char * const Ip = "Ip";
    const char * const Port = "Port";
    const char * const ErrorCode = "ErrorCode";
    const char * const Content = "Content";

    const string TypeLogin = "Login";
    const string TypeChat = "Chat";
    const string TypeLogout = "Logout";
}

#endif
