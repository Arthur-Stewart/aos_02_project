#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include <string>
#include <cstddef>
#include <fstream>
#include <map>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

class Parser
{
	private:

	public:
		std::string config;

		int line_num; 
		int num_nodes;

		std::string line;

		std::unordered_map<int, Node> node_map;

		std::map<int, std::vector<int>> table;

		Parser(const std::string config_file);

        void Parse_Config();
		bool Is_Valid_Line(std::string line);
};

#endif // PARSER_H
