#ifndef INCLUDE_CONFIGREADER_H
#define INCLUDE_CONFIGREADER_H

#include <string>
#include <map>
#include <vector>

class ConfigReader
{
public:
	ConfigReader(const std::string& filename);
	~ConfigReader();
	static std::vector<std::string> SplitArguments(const std::string& line);
	const std::vector<std::string>& GetSection(const std::string& section);

	// trim from start
	static std::string &ltrim(std::string &s);

	// trim from end
	static std::string &rtrim(std::string &s);

	// trim from both ends
	static std::string &trim(std::string &s);
private:
	std::map<std::string, std::vector<std::string>> linesBySection;
};

#endif //INCLUDE_CONFIGREADER_H