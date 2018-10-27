#include "message.h"

Message::Message() : kind("outbound")
{}

Message::Message(std::string kind) : kind(kind)
{}

Message::Message(std::string kind, std::string contents) : kind(kind) , contents(contents)
{}

std::string Message::To_String()
{
	if (contents.empty())
	{
		return kind + " " + std::to_string(source) + " " + std::to_string(origin);
	}
	else
	{
		return kind + " " + std::to_string(source) + " " + std::to_string(origin) + " " + " " + contents ;
	}
}

// To print message contents for debugging 
std::ostream &operator<<(std::ostream &os, Message const &m)
{
	os << "KIND:" << m.kind << std::endl;
	os << "SOURCE:" << m.source;
	os << "ORIGIN:" << m.origin;
	os << "CONTENT:" << m.contents;
	
	return os;
}
