#pragma once

#include <string>

namespace slag {

struct ModuleIdentifier
{
	ModuleIdentifier(const std::string& name, const std::string& instance);

    //! parses a string for module ID
	ModuleIdentifier(const char* id = "");

	ModuleIdentifier& assign(const char* id);
	ModuleIdentifier& assign(const std::string& n = "", const std::string& i = "");
	std::string name;
	std::string instance;
	// std::string library;
    
    //! returns readable format of the module ID: "name[.instance]"
    /*!
        if the ID is incorrect or empty then returns an empty string
    */
	operator std::string ()const;
	bool operator< (const ModuleIdentifier& other)const;
	bool operator== (const ModuleIdentifier& other)const;
};


struct FullModuleIdentifier
{
    FullModuleIdentifier(const std::string& name, const std::string& instance, const std::string& dll);

    //! parses a string for module ID
    FullModuleIdentifier(const char* id = "");

    FullModuleIdentifier& assign(const char* id);
    FullModuleIdentifier& assign(const std::string& n = "", const std::string& i = "", const std::string& d = "");
    ModuleIdentifier module;
    std::string library;

    //! returns readable format of the module ID: "name[.instance]"
    /*!
    if the ID is incorrect or empty then returns an empty string
    */
    operator std::string()const;
    bool operator< (const FullModuleIdentifier& other)const;
    bool operator== (const FullModuleIdentifier& other)const;
};

typedef int PortNumber;

struct PortIdentifier
{
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

struct ConnectionIdentifier
{
    ConnectionIdentifier(const PortIdentifier& from, const PortIdentifier& to);
    //! parses a string for connection ID.
    // ConnectionIdentifier(const std::string& id = "");
    //! returns readable format of the connection: "port -> port"
    /*!
        if the any of the ports are incorrect or empty then returns an empty string
    */
    operator std::string()const;
    
    bool operator< (const ConnectionIdentifier& other)const;
    bool operator== (const ConnectionIdentifier& other)const;
    PortIdentifier from, to;
};

}

namespace std{
    template<>
    struct hash<slag::ModuleIdentifier>
    {
        static hash<string> strhasher;
        size_t operator()(const slag::ModuleIdentifier &m) const
        {
            return strhasher(m.name) ^ strhasher(m.instance);
        }
    };

    template<>
    struct hash<slag::FullModuleIdentifier>
    {
        static hash<slag::ModuleIdentifier> hasher;
        size_t operator()(const slag::FullModuleIdentifier &m) const
        {
            return hasher(m.module) ^ hash<slag::ModuleIdentifier>::strhasher(m.library);
        }
    };
}
