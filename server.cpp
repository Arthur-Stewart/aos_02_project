#include "server.h"
// Help from Beej's Guide to Sockets

void Server::ProcessMessage(const char* buffer)
{
	std::string b(buffer);
	std::istringstream iss(b);
	std::vector<std::string> msg_tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

	std::string kind = msg_tokens[0];
	std::string source = msg_tokens[1];
	std::string brcOrigin = msg_tokens[2]; // broadcast's origin; also, in a converge cast, converge's origin (a leaf)
	std::string convergeDest = msg_tokens[3]; // convergecast's dest/root

	std::string contents;

	std::string convergeCount; // used for convergecast control
	int convergeSum = 0;

	if((kind == "Broadcast") || (kind == "Convergecast"))
	{
		contents = msg_tokens[4]; 
	}

	//std::cout << "Kind: " << kind << std::endl;
	//std::cout << "Source: " << source << std::endl;

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
		//Let's change it so each client has the node map so you can easily look this up 
		serv.children.emplace_back(node_map[std::stoi(source)]);
		//Add nodes to children
	}

	else if (kind == "No")
	{
		//++num_no_message;
		if (++num_no_message == num_discovery_message)
		{
			Message_Handler("Done", serv.parent[0]);
		}
	}
	
	else if (kind == "Done")
	{
		//++num_done_message;
		if(++num_done_message == serv.children.size())
		{
			// parent is empty,i.e. no parent => is root
			if (serv.parent.empty())
			{
				//std::cout << "Finished building tree" << std::endl;\newline
				serv.PrintTree();
				for (const auto&n: serv.children)
				{
					Message_Handler("Finished", n);
				}
				discovered = true;
			}
			else
			{
				Message_Handler("Done", serv.parent[0]);
				//If node has recived done from all of it's children
				//Send done to parent
			}
		}
	}

	else if (kind == "Finished")
	{
		serv.PrintTree();
		for (const auto&n: serv.children)
		{
			Message_Handler("Finished", n);
		}
		discovered = true;
	}

	else if (kind == "Broadcast")
	{
		std::cout << "Received Broadcast" << std::endl;
		std::cout << contents << std::endl;
		test_nums.emplace_back(std::stoi(contents));
		sum += std::stoi(contents);
		Broadcast(contents, node_map[std::stoi(source)]);
		
		// set reach path
		// for each node x, reach path for node i which is not 1-hop neighbor of x,
		// is node j if broadcast msg with origin = i came to x through source = j
		// j is 1-hop neighbor of x
		this->reachPath[std::stoi(brcOrigin)] = std::stoi(source);

		// receiving broadcast from an origin x set value at index x to false, 
		// receiving convergecast intended for converge destination x set value at index x to true
		// here, on reception of broadcast, set converged for index x = brcast origin to false
		this->destConverged[std::stoi(brcOrigin)] = false;
		
		// children vector is empty, i.e. no children => is leaf, start convergecast
		if ( serv.children.empty() )
		{
			convergeCount = std::to_string(1);
			// dest = parent, origin = this leaf node, converge dest/root = broadcast's origin
			Message_Handler("Convergecast", serv.parent[0], convergeCount, serv.node_id, std::stoi(brcOrigin));
			
			// set converged flag for index x = brcast origin to true
			this->destConverged[std::stoi(brcOrigin)] = true;
		}
	}

	else if (kind == "Convergecast")
	{
		// if convergeDest != this node
		// continue ongoing convergecast for a converge destination
		// dest = find through reach path at index = converge dest 
		// origin = unchanged, converge dest/root = unchanged

		if ((std::stoi(convergeDest)) != serv.node_id )
		{
			if (!(this->destConverged[std::stoi(convergeDest)]))
			{
				// if converged flag for destination x is false, increment converge count, then set flag to true
				convergeCount = std::to_string(std::stoi(contents) + 1);
				this->destConverged[std::stoi(convergeDest)] = true;
				Message_Handler("Convergecast", this->node_map[this->reachPath[std::stoi(convergeDest)]], convergeCount, std::stoi(brcOrigin), std::stoi(convergeDest) );
			}
			else
			{
				// else, no change to converge count (in contents)
				Message_Handler("Convergecast", this->node_map[this->reachPath[std::stoi(convergeDest)]], contents, std::stoi(brcOrigin), std::stoi(convergeDest) );
			}
		}
		else
		{
			// converge cast msg reach destination
			convergeSum += std::stoi(contents);
			// convergeSum = number of nodes - 1 => convergecast completed => broadcast completed successfully
			if(convergeSum == (this->num_nodes - 1) )
			{
				this->finBroadcast = true;
			}
		}
		

	}
}

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

	//printf("server: waiting for connections...\n");

	char buffer[1024];

	bool flag = true;

	while(true) 
	{  // main accept() loop
		sin_size = sizeof their_addr;
		newsockfd= accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (newsockfd == -1) 
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		//printf("server: got connection from %s\n", s);
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
			// Generate random numbers
			for (int i = 0; i <= 30; ++i)
			{
				sum += i;
				test_nums.emplace_back(i);
				Broadcast(std::to_string(i), serv.node_id);
			}
			flag = false;
		}

		// How do I end the message service?
		// I'm not sure
		// Khoa: no requirement for termination or end of service, only requirement for notification of broadcast completion
		//std::cout << "Sum: " << std::accumulate(test_nums.begin(), test_nums.end(), 0) << std::endl;

		// finBroadcast can also be used as flag for termination of the service, when needed.
		if(this->finBroadcast)
		{
			// completed a broadcast
			std::cout << "Broadcast completed!" << std::endl;
			std::cout << "Sum: " << sum << std::endl;
			this->finBroadcast = false;
		}

	}  // end server
	sleep(2);
	close(sockfd);
}

void Server::Broadcast(std::string contents, Node ignore)
{
	// Send a message to all neighbors
	// Except that you want to excluse sending it back to the source
	// I'll just overload this function for that case
	
	//std::cout << "Broadcasting " << contents << std::endl;
	
	if(!serv.parent.empty())
	{
		if (ignore.node_id != serv.parent[0].node_id)
		{
			Message_Handler("Broadcast", serv.parent[0], contents);
		}
	}
	for (const auto& n: serv.children)
	{
		if (ignore.node_id != n.node_id)
		{
			Message_Handler("Broadcast", n, contents);
		}
	}
}

void Server::Broadcast(std::string contents)
{
	// Send a message to all neighbors
	// Except that you want to excluse sending it back to the source
	// I'll just overload this function for that case
	std::cout << "Broadcasting " << contents << std::endl;
	Broadcast(contents, serv);
}

// Khoa: I add 2 overloaded broadcast function to include origin

void Server::Broadcast(std::string contents, Node ignore, int origin)
{
	// Send a message to all neighbors
	// Except that you want to excluse sending it back to the source
	// I'll just overload this function for that case
	
	//std::cout << "Broadcasting " << contents << std::endl;
	
	if(!serv.parent.empty())
	{
		if (ignore.node_id != serv.parent[0].node_id)
		{
			Message_Handler("Broadcast", serv.parent[0], contents, origin);
		}
	}
	for (const auto& n: serv.children)
	{
		if (ignore.node_id != n.node_id)
		{
			Message_Handler("Broadcast", n, contents, origin);
		}
	}
}

void Server::Broadcast(std::string contents, int origin)
{
	// Send a message to all neighbors
	// Except that you want to excluse sending it back to the source
	// I'll just overload this function for that case
	std::cout << "Broadcasting " << contents << std::endl;
	Broadcast(contents, serv, origin);
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

// Khoa: I add 2 overloaded Message_Handler functions
// 1 to work with Broadcast with origin, and 1 to work with Convergecast
void Server::Message_Handler(std::string type, Node destination, std::string contents, int origin)
{
	Message pack(type, contents);
	pack.source = serv.node_id;
	pack.origin = origin;
	pack.convergeDest = -1;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}

void Server::Message_Handler(std::string type, Node destination, std::string contents, int origin, int convergeDest)
{
	Message pack(type, contents);
	pack.source = serv.node_id;
	pack.origin = origin;
	pack.convergeDest = convergeDest;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}


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

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

