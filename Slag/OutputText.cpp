#include "OS_dependent.h"

#include <sstream>

void terminate_output_text()
{
}

void configure_output_text( const std::vector<std::string>& )
{
}

void handle_output_text( const std::string&, const char*)
{
}

void handle_statistics( const std::string&, double, double, double, const std::map<PortNumber, size_t>&)
{
}

std::vector<std::string> split_to_argv(const std::string& line)
{
	std::istringstream iss(line);
	std::vector<std::string> argv;
	std::string param;
	while (iss >> param)
		argv.emplace_back(param);
	return argv;
}
