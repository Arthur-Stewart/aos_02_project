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
		return kind + " " + std::to_string(source) + " " + std::to_string(origin) + " " + std::to_string(convergeDest) ;
	}
	else
	{
		return kind + " " + std::to_string(source) + " " + std::to_string(origin) + " " + std::to_string(convergeDest) + " " + contents ;
	}
}

std::ostream &operator<<(std::ostream &os, Message const &m)
{
	std::cout << "KIND:" << m.kind << std::endl;
	std::cout << "SOURCE:" << m.source;
	
	// Khoa:
	// I added the  two lines below when adding new members in Message class
	// but I am not sure what your intended use for this operator overloading was, so I commented them out.
	// std::cout << "ORIGIN:" << m.origin;
	// std::cout << "CONVERGE DESTINATION:" << m.convergeDest;
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
