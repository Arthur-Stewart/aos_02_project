#include "message.h"

Message::Message()
{
	this -> kind = "outbound";
}

Message::Message(std::string kind)
{
	this -> kind = kind;
}

Message::Message(std::string kind, std::string contents) : kind(kind) , contents(contents)
{
}

std::string Message::To_String()
{
	if (contents.empty())
	{
		return kind + " " + std::to_string(source);
	}
	else
	{
		return kind + " " + std::to_string(source) + " " + contents;
	}
}

std::ostream &operator<<(std::ostream &os, Message const &m)
{
	std::cout << "KIND:" << m.kind << std::endl;
	std::cout << "SOURCE:" << m.source;
	//std::cout << m.source << std::endl;
//	for (const auto& i: m.path)
//	{
//		std::cout << i << " "; 	
//	}
//	std::cout << std::endl;
//	std::cout << "VISITED ";
//	for (const auto& i: m.visited)
//	{
//		std::cout << i << " "; 	
//	}
//	std::cout << std::endl;
	
	return os;
}
