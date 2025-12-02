#pragma once

#include <string>
#include <unordered_map>

class Placeholders
{
public:
	static void SetPlaceholders(const std::string& buildMode, const std::string& installPrefix, const std::string& modulePathname);
#ifdef WIN32
	static void SetPlaceholders(const std::string& buildMode, const std::string& installPrefix, const std::string& modulePathname, const std::string& compilerPath);
#endif // 
	static std::string GetPlaceholder(std::string key);
private:
	static std::unordered_map<std::string, std::string> s_Placeholders;
};
