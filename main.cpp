#include "node.h"
#include "client.h"
#include "message.h"
#include "parser.h"
#include "server.h"

#include <algorithm>
#include <cctype> 
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>

// Break the problem into pieces
// After setting up the topology you need a distributed algorithm that finds the k hop neights (communication through sockets)

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::cerr << "usage: ./main config node_id" << std::endl; 
	 	return -1;
	}

	Parser p1(argv[1]);
	p1.Parse_Config();

	Node process_node = p1.node_map[std::stoi(argv[2])];
	std::cout << "node_id_process " << process_node.node_id << std::endl;

	Server s1(process_node);
	s1.num_nodes = p1.num_nodes;
	std::thread t1(&Server::Listen, s1);

	sleep(3); // To let all servers get setup

	// Let node 0 be the source discovery node
	if (process_node.node_id == 0)
	{
		//Send a message to all neighbors
		//I want some kind of tree node class
		
		Message out("Discovery");
		out.source = process_node.node_id;

		//std::cout << out << std::endl;
		
		//std::cout << "Test: " <<  out.To_String() << std::endl;

		// Send message to one hop neighbors
		for (const auto& one_hop: process_node.one_hop_neighbors)
		{
			Message out_hop = out;
			Client c1(process_node, one_hop);
			c1.SendMessage(out_hop);

			// Should I close the socket? Should I multi-thread this?
		}
	}

	t1.join();

	// How do I know when to stop everything?
}

