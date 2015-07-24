#pragma once
#include <string>
#include <sstream>

struct ModuleIdentifier
{
	ModuleIdentifier(const std::string& name = "", const std::string& instance = "", const std::string& dll = "");

	ModuleIdentifier(const char* id);

	ModuleIdentifier& assign(const char* id);
	ModuleIdentifier& assign(const std::string& n = "", const std::string& i = "", const std::string& d = "");
	std::string name;
	std::string instance;
	std::string dll;
	operator std::string ()const;
	bool operator< (const ModuleIdentifier& other)const;
};

typedef int PortNumber;

class PortIdentifier
{
public:
	ModuleIdentifier module;
	PortNumber port;
	PortIdentifier(const ModuleIdentifier& m, PortNumber p = 0);
	PortIdentifier(const char* id);
	bool operator< (const PortIdentifier& other)const;
	operator std::string ()const;
};

