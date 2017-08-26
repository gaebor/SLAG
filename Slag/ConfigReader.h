#ifndef INCLUDE_CONFIGREADER_H
#define INCLUDE_CONFIGREADER_H

#include <string>
#include <map>
#include <vector>

class ConfigReader
{
public:
	ConfigReader(std::ifstream& f);
	~ConfigReader();
	const std::vector<std::string>& GetSection(const std::string& section);

	static std::string& trim(std::string& s);
	static std::string trim1(const std::string& s);
private:
	std::map<std::string, std::vector<std::string>> linesBySection;
};

#endif //INCLUDE_CONFIGREADER_H
