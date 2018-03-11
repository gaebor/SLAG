#ifndef INCLUDE_MODULE_IDENTIFIER_H
#define INCLUDE_MODULE_IDENTIFIER_H

#include <string>

struct ModuleIdentifier
{
	ModuleIdentifier(const std::string& name = "", const std::string& instance = "", const std::string& dll = "");

    //! parses a string for module ID
	ModuleIdentifier(const char* id);

	ModuleIdentifier& assign(const char* id);
	ModuleIdentifier& assign(const std::string& n = "", const std::string& i = "", const std::string& d = "");
	std::string name;
	std::string instance;
	std::string library;
    
    //! returns readable format of the module ID: "name[.instance]"
    /*!
        if the ID is incorrect or empty then returns an empty string
    */
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
    //! parses a string for port ID
	PortIdentifier(const std::string& id = "");
	bool operator< (const PortIdentifier& other)const;
	bool operator== (const PortIdentifier& other)const;
    //! returns readable format of the port "module:port"
    /*!
        if the port is incorrect or empty then returns an empty string
    */
	operator std::string ()const;
};

class ConnectionIdentifier
{
public:
    ConnectionIdentifier(const PortIdentifier& from, const PortIdentifier& to);
    //! parses a string for connection ID.
    ConnectionIdentifier(const std::string& id = "");
    //! returns readable format of the connection: "port -> port"
    /*!
        if the any of the ports are incorrect or empty then returns an empty string
    */
    operator std::string()const;
    
    bool operator< (const ConnectionIdentifier& other)const;
    bool operator== (const ConnectionIdentifier& other)const;
    PortIdentifier from, to;
};
#endif //INCLUDE_MODULE_IDENTIFIER_H