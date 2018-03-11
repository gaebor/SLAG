#ifndef INCLUDE_MODULE_IDENTIFIER_H
#define INCLUDE_MODULE_IDENTIFIER_H

#include <string>

struct ModuleIdentifier
{
	ModuleIdentifier(const std::string& name = "", const std::string& instance = "", const std::string& dll = "");

	ModuleIdentifier(const char* id);

	ModuleIdentifier& assign(const char* id);
	ModuleIdentifier& assign(const std::string& n = "", const std::string& i = "", const std::string& d = "");
	std::string name;
	std::string instance;
	std::string library;
	operator std::string ()const;
	bool operator< (const ModuleIdentifier& other)const;
	bool operator== (const ModuleIdentifier& other)const;
};

typedef int PortNumber;

class PortIdentifier
{
public:
	ModuleIdentifier module;
	PortNumber port;
	PortIdentifier(const ModuleIdentifier& m, PortNumber p = 0);
	PortIdentifier(const std::string& id = "");
	bool operator< (const PortIdentifier& other)const;
	bool operator== (const PortIdentifier& other)const;
	operator std::string ()const;
};

class ConnectionIdentifier
{
public:
    ConnectionIdentifier(const PortIdentifier& from, const PortIdentifier& to);
    ConnectionIdentifier(const std::string& id = "");
    operator std::string()const;
    
    bool operator< (const ConnectionIdentifier& other)const;
    bool operator== (const ConnectionIdentifier& other)const;
    PortIdentifier from, to;
};
#endif //INCLUDE_MODULE_IDENTIFIER_H