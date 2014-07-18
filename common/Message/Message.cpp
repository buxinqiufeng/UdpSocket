#include "Message.h"

string Message::INVALID_VALUE = "invalid_value";
string Message::ROOT_ELEMENT = "root";

Message::~Message()
{
    if(null != mXmlDoc)
    {
        delete mXmlDoc;
    }
}

void Message::SetValue(const char* attr, const char* value)
{
    if(null != mRootElement)
    {
        mRootElement->SetAttribute(attr, value);
    }
}

string Message::GetValue(const char* attr)
{
    if(null != mRootElement)
    {
        return mRootElement->Attribute(attr);
    }
    return null;
}

void Message::SetMessageType(const char* type)
{
    mTag = type;
    mRootElement = new TiXmlElement(ROOT_ELEMENT);
    mXmlDoc->LinkEndChild(mRootElement);
    isElementNew = true;
    SetValue(MessageTag::Type, type);
}

void Message::SetMessageType(const string type)
{
    SetMessageType(type.c_str());
}

string Message::GetMessageType()
{
    return GetValue(MessageTag::Type);
}

string& Message::GenerateMessage()
{
    mStrMessage = "";
    if(null != mXmlDoc)
    {
        TiXmlPrinter printer;
        mXmlDoc->Accept(&printer);
        mStrMessage = printer.CStr();
    }
    return mStrMessage;
}

bool Message::ParaseMessage(const char* msg)
{
    if(null != mXmlDoc)
    {
        if(mXmlDoc->Parse(msg))
        {
            mRootElement = mXmlDoc->RootElement();
            return true;
        }
    }
    return false;

}
