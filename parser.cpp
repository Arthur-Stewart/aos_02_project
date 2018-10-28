#include "parser.h"

Parser::Parser(const std::string config) : config(config), line_num(0), num_nodes(0)
{}

void Parser::Parse_Config()
{
	std::ifstream in(config);
	while (std::getline(in, line))
	{
		if (Is_Valid_Line(line))
		{
			// FirstLine
			if (line_num == 0)
			{
				num_nodes = std::stoi(line);	
			}
			// First n
			else if (line_num < num_nodes + 1)
			{
				std::istringstream iss(line);
				std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};
				int node_id = line_num - 1;

				std::string host = tokens[0] + ".utdallas.edu";
				std::string port = tokens[1];
				node_map[node_id] = Node(node_id, host, port);

				for (std::vector<std::string>::iterator it = tokens.begin() + 2; it != tokens.end(); ++it)
				{
					if (((*it).find('#') != std::string::npos))
					{
						break;
					}

					table[node_id].emplace_back(std::stoi(*it));
				}
			}
			
			line_num++;
		}
	}

	for (int i = 0; i < num_nodes; ++i)
	{
		for (auto &n: table[i])
		{
			node_map[i].Add_One_Hop_Neighbor(node_map[n]);
		}
	}
}
bool Parser::Is_Valid_Line(std::string line)
{
	std::istringstream iss(line);
	std::string first;
	iss >> first;
	if (first.empty())
	{
		return false;
	}
	for (auto it: first)
	{
		if (it == '#')
		{
			return false;
		}
	}
	return true;
}
