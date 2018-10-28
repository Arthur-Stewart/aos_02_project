#include "message.h"

Message::Message() : kind("outbound"), origin(-1)
{}

Message::Message(std::string kind) : kind(kind), origin(-1)
{}

Message::Message(std::string kind, std::string contents) : kind(kind), origin(-1), contents(contents)
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
	os << "SOURCE:" << m.source << std::endl;
	if (!m.contents.empty())
	{
		os << "CONTENTS:" << m.contents << std::endl;
	}
	if (m.origin != -1)
	{
		os << "ORIGIN:" << m.origin << std::endl;
	}
	
	return os;
}
