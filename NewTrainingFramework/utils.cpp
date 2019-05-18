#include "stdafx.h"
#include "utils.h"
#include <fstream>
#include <sstream>

std::string Utils::Trim(const std::string& str)
{
	if (str == "")
		return "";
	const char* beg = str.c_str();
	while (strchr(" \n\t", *beg))
		beg++;
	const char* end = str.c_str() + str.size();
	end--;
	while (strchr(" \n\t", *end))
		end--;
	end++;
	return std::string(beg, end);
}

std::string Utils::ReadFile(const std::string& path)
{
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

std::string Utils::AddLineNumbers(const std::string& str)
{
	int line = 1, pos = 0, newpos;
	std::string res = "";
	while ((newpos = str.find('\n', pos)) != std::string::npos)
	{
		res += std::to_string(line) + ". ";
		res += str.substr(pos, newpos - pos + 1);
		pos = newpos + 1;
		line++;
	}
	res += std::to_string(line) + ". ";
	res += str.substr(pos);
	return res;
}

bool Utils::StartsWith(const std::string& str, const std::string& prefix)
{
	if (prefix.size() > str.size())
		return false;
	for (int i = 0; i < prefix.size(); i++)
	{
		if (prefix[i] != str[i])
			return false;
	}
	return true;
}

bool Utils::EndsWith(const std::string& str, const std::string& suffix)
{
	if (suffix.size() > str.size())
		return false;
	for (int i = 0; i < suffix.size(); i++)
	{
		if (suffix[i] != str[i - suffix.size() + str.size()])
			return false;
	}
	return true;
}
