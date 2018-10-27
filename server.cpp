#include "server.h"
// Help from Beej's Guide to Sockets

int Server::Listen()
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	//std::cout << "Listening port: " << serv.port << std::endl;

	if ((rv = getaddrinfo(NULL, serv.port.c_str(), &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		error_num = 1;
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL)  
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	char buffer[1024];
	bool flag = true;

	while(true) // main accept() loop
	{  
		sin_size = sizeof their_addr;
		newsockfd= accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (newsockfd == -1) 
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        memset(buffer, 0, 1024);
        int read_rtn = read(newsockfd, buffer, 1023);
		if (read_rtn >= 0)
		{
			ProcessMessage(buffer);
		} 

		// Figure out if tree is finished
		if (flag && discovered)
		{
			// Add test numbers to vector
			for (int i = 0; i <= 30; ++i)
			{
				sum += i;
				test_nums.emplace_back(i);
				message_queue.emplace(std::to_string(i));
			}

			// Process first message in the queue
			Broadcast(message_queue.front(), serv);
			message_queue.pop();

			flag = false;
		}

		std::cout << "Sum: " << sum << std::endl;

	}  // end server
	sleep(2);
	close(sockfd);
}

void Server::ProcessMessage(const char* buffer)
{
	std::string b(buffer);
	std::istringstream iss(b);
	std::vector<std::string> msg_tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

	std::string kind = msg_tokens[0];
	std::string source = msg_tokens[1];
	std::string origin = msg_tokens[2]; // broadcast's origin
	
	std::string contents;

	if((kind == "Broadcast") || (kind == "Convergecast"))
	{
		contents = msg_tokens[3]; 
	}

	if (kind == "Discovery")
	{
		if (serv.parent.empty())
		{
			serv.parent.emplace_back(node_map[std::stoi(source)]);
			Message_Handler("Parent", source);
					
			for (const auto& one_hop: serv.one_hop_neighbors)
			{
				if (serv.parent[0].node_id != one_hop.node_id)
				{
					++num_discovery_message;
				}
			}

			if (num_discovery_message == 0)
			{
				Message_Handler("Done", serv.parent[0]);
			}

			//Sends a Discovery message to each of it's neighbors
			for (const auto& one_hop: serv.one_hop_neighbors)
			{
				if (serv.parent[0].node_id != one_hop.node_id)
				{
					Message_Handler("Discovery", one_hop);
				}
			}
		}
		//PARENT field already set send no
		else
		{
			Message_Handler("No", source);
		}
	}
	
	else if (kind == "Parent")
	{
		serv.children.emplace_back(node_map[std::stoi(source)]);
	}

	else if (kind == "No")
	{
		if (++num_no_message == num_discovery_message)
		{
			Message_Handler("Done", serv.parent[0]);
		}
	}
	
	else if (kind == "Done")
	{
		if(++num_done_message == serv.children.size())
		{
			// parent is empty,i.e. no parent => is root
			if (serv.parent.empty())
			{
				//std::cout << "Finished building tree" << std::endl;\newline
				serv.PrintTree();

				// Add tree_neighbors (unrooted tree)
				serv.tree_neighbors = serv.children;

				serv.PrintTreeNeighbors();

				for (const auto&n: serv.children)
				{
					Message_Handler("Finished", n);
				}
				discovered = true;
			}
			else //If node has recived done from all of its children send done to parent
			{
				Message_Handler("Done", serv.parent[0]);
			}
		}
	}

	else if (kind == "Finished")
	{
		// Add tree_neighbors (unrooted tree)
		for (auto &v: serv.parent)
		{
			serv.tree_neighbors.emplace_back(v);
		}
		for (auto &v: serv.children)
		{
			serv.tree_neighbors.emplace_back(v);
		}

		serv.PrintTree();

		serv.PrintTreeNeighbors();
		
		for (const auto&n: serv.children)
		{
			Message_Handler("Finished", n);
		}
		discovered = true;
	}

	else if (kind == "Broadcast")
	{
		std::cout << "Received Broadcast: " << contents << std::endl;

		test_nums.emplace_back(std::stoi(contents));
		sum += std::stoi(contents);

		//Set parent_map
		parent_map[std::stoi(origin)] = std::stoi(source);

		Broadcast(contents, serv, std::stoi(origin));
	}

	//else if (kind == "Convergecast")
	//{
	//	// if convergeDest != this node
	//	// continue ongoing convergecast for a converge destination
	//	// dest = find through reach path at index = converge dest 
	//	// origin = unchanged, converge dest/root = unchanged

	//	if ((std::stoi(convergeDest)) != serv.node_id )
	//	{
	//		if (!(this->destConverged[std::stoi(convergeDest)]))
	//		{
	//			// if converged flag for destination x is false, increment converge count, then set flag to true
	//			convergeCount = std::to_string(std::stoi(contents) + 1);
	//			this->destConverged[std::stoi(convergeDest)] = true;
	//			Message_Handler("Convergecast", this->node_map[this->reachPath[std::stoi(convergeDest)]], convergeCount);
	//		}
	//		else
	//		{
	//			// else, no change to converge count (in contents)
	//			Message_Handler("Convergecast", this->node_map[this->reachPath[std::stoi(convergeDest)]], contents);
	//		}
	//	}
	//	else
	//	{
	//		// converge cast msg reach destination
	//		convergeSum += std::stoi(contents);
	//		// convergeSum = number of nodes - 1 => convergecast completed => broadcast completed successfully
	//		if(convergeSum == (this->num_nodes - 1) )
	//		{
	//			this->finBroadcast = true;
	//		}
	//	}
	//}
}

//Broadcast operation for origin node
void Server::Broadcast(std::string contents, Node ignore)
{
	std::cout << "Broadcasting: " << contents << std::endl;
	// Send a message to all neighbors
	for (const auto& n: serv.tree_neighbors)
	{
		Message_Handler("Broadcast", n, contents, ignore.node_id);
	}
}

// KEY BROADCAST OPERATION
void Server::Broadcast(std::string contents, Node ignore, int origin)
{
	// Send a message to all neighbors except ignore (previous sender)
	for (const auto &dest: serv.tree_neighbors)
	{
		if (ignore.node_id != dest.node_id)
		{
			Message_Handler("Broadcast", dest, contents, origin);
		}
	}
}

// KEY MESSAGE_HANDLER OPERATION
void Server::Message_Handler(std::string type, Node destination, std::string contents, int origin)
{
	Message pack(type, contents);
	pack.source = serv.node_id;
	pack.origin = origin;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}

void Server::Message_Handler(std::string type, Node destination, std::string contents)
{
	Message pack(type, contents);
	pack.source = serv.node_id;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}

void Server::Message_Handler(std::string type, std::string destination)
{
	Message_Handler(type, node_map[std::stoi(destination)]);
}

void Server::Message_Handler(std::string type, Node destination)
{
	Message pack(type);
	pack.source = serv.node_id;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}

//Server Constructors/Destructor

Server::Server(Node& serv)
{
	this -> serv = serv;
	discovered = false;
	num_terminate_messages = 0;
}

Server::Server(Node& serv, std::unordered_map<int, Node> node_map) : serv(serv), node_map(node_map), num_discovery_message(0), num_no_message(0), num_done_message(0), num_terminate_messages(0), discovered(false), finBroadcast(false), sum(0)
{
	num_nodes = node_map.size();
	reachPath = new int[num_nodes];
	memset(reachPath, -1, num_nodes);
	destConverged = new bool[num_nodes];
	memset(destConverged, false, num_nodes);
}

Server::~Server()
{
	delete [] reachPath;
	delete [] destConverged;
}



// SOCKET HELPERS

void *Server::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	//waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

