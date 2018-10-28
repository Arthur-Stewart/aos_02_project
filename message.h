#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>

class Message
{
	public:
		std::string kind;
		int source; // sending node
		int origin; // where the msg first generated, used for broadcast - convergecast
		int destination; // receiving node
		
		std::string contents;

		Message();
		Message(std::string kind);
		Message(std::string kind, std::string contents);

		std::string To_String();
		
		friend std::ostream &operator<<(std::ostream &os, Message const &m); 
};

#endif // MESSAGE_H
