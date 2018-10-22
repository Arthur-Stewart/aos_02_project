#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>

class Message
{
	public:
		std::string kind;
		int source;

		std::vector<int> path;
		std::vector<int> visited;
		int destination;

		std::string contents;

		Message();
		Message(std::string kind);
		Message(std::string kind, std::string contents);

		std::string To_String();
		friend std::ostream &operator<<(std::ostream &os, Message const &m); 
		//int hop_number; // You don't need the hop number because the size of path tells you the hop number
};

#endif // MESSAGE_H
