#ifndef NODE_H
#define NODE_H

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

class Node
{
	public:
		int node_id;
		std::string hostname;
		std::string port;
		std::vector <Node> one_hop_neighbors;	

		std::vector <Node> parent; 
		std::vector <Node> children;

		Node();
		Node(int node_id, std::string hostname, std::string port);

		void PrintTree();

		void Add_One_Hop_Neighbor(const Node& neighbor);
		friend std::ostream &operator<<(std::ostream &os, Node const &n); 
};

#endif // NODE_H
