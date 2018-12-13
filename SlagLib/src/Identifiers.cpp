#include "slag/Identifiers.h"

#include <sstream>

namespace std
{
    hash<slag::ModuleIdentifier> hash<slag::FullModuleIdentifier>::hasher;

    hash<string> hash<slag::ModuleIdentifier>::strhasher;
}

namespace slag {

ModuleIdentifier::ModuleIdentifier( const std::string& n, const std::string& i /*= ""*/)
{
	assign(n,i);
}

ModuleIdentifier::ModuleIdentifier( const char* id )
{
	assign(id);
}


ModuleIdentifier::operator std::string() const
{
    std::ostringstream oss;
    oss << name;

    if (!instance.empty())
        oss << '.' << instance;

    return oss.str();
}

bool ModuleIdentifier::operator<(const ModuleIdentifier& other) const
{
    return (name == other.name) ? instance < other.instance : name < other.name;
}

bool ModuleIdentifier::operator==(const ModuleIdentifier& other) const
{
    return (name == other.name) && instance == other.instance;
}

ModuleIdentifier& ModuleIdentifier::assign(const std::string& n, const std::string& i)
{
    name = n;
    instance = i;
    return *this;
}

ModuleIdentifier& ModuleIdentifier::assign(const char* id)
{
    std::string n, i;
    std::string idStr = id;

    auto instanceSeparator = idStr.rfind('.');

    if (instanceSeparator != std::string::npos)
    {
        n = idStr.substr(0, instanceSeparator);
        i = idStr.substr(instanceSeparator + 1);
    }
    else {
        n = idStr;
    }
    return assign(n, i);
}

FullModuleIdentifier::FullModuleIdentifier(const std::string & n, const std::string & i, const std::string & d)
{
    assign(n, i, d);
}

FullModuleIdentifier::FullModuleIdentifier(const char * id)
{
    assign(id);
}

FullModuleIdentifier & FullModuleIdentifier::assign(const char * id)
{
    std::string d, idStr = id;
    auto dllSeparator = idStr.rfind('/');
    if (dllSeparator != std::string::npos)
    {
        library = idStr.substr(0, dllSeparator);
        ++dllSeparator;
    }
    else
    {
        dllSeparator = 0;
        library.clear();
    }

    module.assign(id + dllSeparator);
    return *this;
}

FullModuleIdentifier & FullModuleIdentifier::assign(const std::string & n, const std::string & i, const std::string & d)
{
    module.assign(n, i);
    library = d;
    return *this;
}


//! returns readable format of the module ID: "name[.instance]"
/*!
if the ID is incorrect or empty then returns an empty string
*/

FullModuleIdentifier::operator std::string() const
{
    std::string result = library;
    if (!result.empty())
        result += "/";
    result += (std::string)module;
    return result;
}

bool FullModuleIdentifier::operator<(const FullModuleIdentifier& other) const
{
    return (library == other.library) ? module < other.module : library < other.library;
}

bool FullModuleIdentifier::operator==(const FullModuleIdentifier& other) const
{
    return (library == other.library) && module == other.module;
}

PortIdentifier::PortIdentifier( const ModuleIdentifier& m, PortNumber p /*= 0*/ )
:	module(m), port(p)
{

}

PortIdentifier::PortIdentifier( const std::string& idStr)
    : module(""), port(0)
{
	auto const portSeparator = idStr.find(':');
    if (portSeparator != idStr.rfind(':'))
        return;

    if (portSeparator != std::string::npos)
	{
		port = atoi(idStr.substr(portSeparator+1).c_str());
		module = ModuleIdentifier(idStr.substr(0,portSeparator).c_str());
	}else
		module = ModuleIdentifier(idStr.c_str());
}

bool PortIdentifier::operator<( const PortIdentifier& other ) const
{
    return (module == other.module) ? port < other.port : module < other.module;
}

bool PortIdentifier::operator==(const PortIdentifier& other) const
{
    return module == other.module && port == other.port;
}

PortIdentifier::operator std::string() const
{
    const std::string moduleStr(module);
    if (moduleStr.empty())
        return "";
    else
    {
        std::ostringstream oss;
        oss << (std::string)module << ':' << port;
        return oss.str();
    }
}

ConnectionIdentifier::ConnectionIdentifier(const PortIdentifier & from, const PortIdentifier & to)
: from(from), to(to)
{
}

//ConnectionIdentifier::ConnectionIdentifier(const std::string& c)
//: from(), to()
//{
//    if (c.find("->") == std::string::npos || c.find("->") != c.rfind("->"))
//        return;
//
//    from = PortIdentifier(ConfigReader::trim1(c.substr(0, c.find("->"))));
//    to = PortIdentifier(ConfigReader::trim1(c.substr(c.find("->") + 2)));
//
//}

ConnectionIdentifier::operator std::string() const
{
    const std::string fromStr = from;
    const std::string toStr = to;
    std::string result;

    if (!fromStr.empty() && !toStr.empty())
        result = fromStr + " -> " + toStr;

    return result;
}

bool ConnectionIdentifier::operator <(const ConnectionIdentifier& other) const
{
    return (from == other.from) ? to < other.to : from < other.from;
}

bool ConnectionIdentifier::operator== (const ConnectionIdentifier& other) const
{
    return (from == other.from) && to == other.to;
}

}
