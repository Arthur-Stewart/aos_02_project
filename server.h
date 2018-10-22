#ifndef SERVER_H
#define SERVER_H

#include "node.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <algorithm>
#include <string>
#include <cstddef>
#include <fstream>
#include <map>
#include <numeric>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

#define BACKLOG 10000 // how many pending connections queue will hold

class Server
{
	public:
		//Socket variables
		int sockfd, newsockfd;  // listen on sock_fd, new connection on newsockfd
		struct addrinfo hints, *servinfo, *p;
		struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		struct sigaction sa;
		int yes=1;
		char s[INET6_ADDRSTRLEN];
		int rv;
		int error_num;

		//From project 1
		std::map<int, int> k_hop_map;
		int num_nodes;
		std::set<int> terminated_nodes;
		int num_terminate_messages;

		//Relevant to project 2
		Node serv;
		std::unordered_map<int, Node> node_map;

		size_t num_discovery_message;
		size_t num_no_message;
		size_t num_done_message;
		bool discovered;

		std::vector<int> test_nums;

		void Message_Handler(std::string type, std::string destination);
		void Message_Handler(std::string type, Node destination);

		void Message_Handler(std::string type, Node destination, std::string contents);

		void Broadcast(std::string contents);
		void Broadcast(std::string contents, Node ignore);
		//void Broadcast(std::string contents, std::string ignore)

		Server(Node& serv);
		Server(Node& serv, std::unordered_map<int, Node> node_map);
		
		int Listen();
		void ProcessMessage(const char* buffer);

		void *get_in_addr(struct sockaddr *sa);
};

void sigchld_handler(int s);

#endif // SERVER_H
