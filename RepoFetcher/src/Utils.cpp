#include "Utils.hpp"
#include <filesystem>
#include <iostream>
#include <regex>
#include <fmt/core.h>
#include "Placeholders.hpp"

#ifdef WIN32
const std::string Utils::s_SystemName = "windows";
#elif defined(__linux__)
const std::string Utils::s_SystemName = "linux";
#elif defined(__FreeBSD__)
const std::string Utils::s_SystemName = "freebsd";
#endif

std::string Utils::GetAbsoluteLocation(const std::string& path)
{
	if (path.starts_with("."))
	{
		std::filesystem::path absolutePath = std::filesystem::absolute(path);
		std::cout << absolutePath.string() << "\n";
		return absolutePath.string();
	}
	else
		return path;
}

std::string Utils::ProcessFlag(const std::string& flag)
{
	std::string input = flag;
	std::string placeholder;
	std::string treatedFlag = flag;
	std::regex re(R"(\{([^}]*)\})");
	std::smatch match;

	if (std::regex_search(input, match, re)) {
		placeholder = match[1];   // "install_prefix"
		treatedFlag = std::regex_replace(input, re, "{}");
		std::string finalFlag = fmt::format(fmt::runtime(treatedFlag), Placeholders::GetPlaceholder(placeholder));
		return finalFlag;
	}
	
	return flag;
}
