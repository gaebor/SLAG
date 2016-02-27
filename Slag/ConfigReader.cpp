#include "ConfigReader.h"

#include <windows.h>
#include <shellapi.h>

#include <fstream>
#include <iostream>

// @see http://stackoverflow.com/a/217605
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

std::string & ConfigReader::ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

std::string & ConfigReader::rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

std::string & ConfigReader::trim(std::string &s)
{
	return ltrim(rtrim(s));
}


static bool IsComment(const std::string& str)
{
	return str.size() == 0 || str[0] == '#';
}

static std::string GetSectionName(const std::string& str)
{
	if (!IsComment(str) && (str.size() > 2 && str[0] == '[' && str[str.size() - 1] == ']'))
	{
		return ConfigReader::trim(str.substr(1, str.size() - 2));
	}else
		return "";
}

ConfigReader::ConfigReader(const std::string& filename)
{
	std::ifstream f(filename);
	std::string line;

	if (f.good())
	{
		std::string section;
		while (f.good())
		{
			auto thisSection = section;
			while (f.good() && (thisSection = GetSectionName(line)).empty())
			{
				if (!line.empty() && ! IsComment(line))
					linesBySection[section].push_back(line);
				getline(f, line);
				trim(line);
			}
			section = thisSection;
			getline(f, line);
			trim(line);
		}
	}
}


ConfigReader::~ConfigReader()
{
}

const std::vector<std::string>& ConfigReader::GetSection(const std::string& section)
{
	return linesBySection[section];
}

std::vector<std::string> ConfigReader::SplitArguments(const std::string& line)
{
	int argc;
	std::wstring wline(line.begin(), line.end());
	std::vector<std::string> argv;

	auto szArglist = ::CommandLineToArgvW(wline.c_str(), &argc);
	if (szArglist == NULL)
		return argv;

	for (; *szArglist != NULL; ++szArglist)
	{
		argv.emplace_back(*szArglist, *szArglist + wcslen(*szArglist));
	}
	return argv;
}
