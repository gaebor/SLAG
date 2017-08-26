#include "ModuleIdentifier.h"


ModuleIdentifier::ModuleIdentifier( const std::string& n, const std::string& i /*= ""*/, const std::string& d /*= ""*/ )
{
	assign(n,i,d);
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

bool ModuleIdentifier::operator<( const ModuleIdentifier& other ) const
{
	if (name != other.name)
		return name < other.name;
	else
		return instance < other.instance;
}

ModuleIdentifier& ModuleIdentifier::assign( const std::string& n, const std::string& i /*= ""*/, const std::string& d /*= ""*/ )
{
	name = n;
	instance = i;
	library = d;
	return *this;
}

ModuleIdentifier& ModuleIdentifier::assign( const char* id )
{
	std::string n,i,d;
	std::string idStr = id;
	auto dllSeparator = idStr.find('/');
	if (dllSeparator != idStr.rfind('/'))
	{
		//invalid syntax!
	}
	if (dllSeparator != std::string::npos)
	{
		d = idStr.substr(0, dllSeparator);
		++dllSeparator;
	}else
		dllSeparator = 0;

	auto instanceSeparator = idStr.find('.', dllSeparator);
	if (instanceSeparator != idStr.rfind('.'))
	{
		//invalid syntax!
	}

	if (instanceSeparator != std::string::npos)
	{
		n = idStr.substr(dllSeparator, instanceSeparator-dllSeparator);
		i = idStr.substr(instanceSeparator+1);
	}else{
		n = idStr.substr(dllSeparator);
	}
	return assign(n, i,d);
}


PortIdentifier::PortIdentifier( const ModuleIdentifier& m, PortNumber p /*= 0*/ )
:	module(m), port(p)
{

}

PortIdentifier::PortIdentifier( const char* id ) : module(""), port(0)
{
	std::string idStr = id;

	auto portSeparator = idStr.find(':');
	if (portSeparator != idStr.rfind(':'))
	{
		//parse error, more than one ':'
	}
	if (portSeparator != std::string::npos)
	{
		port = atoi(idStr.substr(portSeparator+1).c_str());
		module = ModuleIdentifier(idStr.substr(0,portSeparator).c_str());
	}else
		module = ModuleIdentifier(id);	
}

bool PortIdentifier::operator<( const PortIdentifier& other ) const
{
	if (module < other.module)
		return true;
	else if (other.module < module)
		return false;
	else
		return port < other.port;
}

PortIdentifier::operator std::string() const
{
	std::ostringstream oss;
	oss << (std::string)module << ':' << port;
	return oss.str();
}
