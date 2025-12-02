#include "Placeholders.hpp"
#include <algorithm> // Adicione este include para std::transform

std::unordered_map<std::string, std::string> Placeholders::s_Placeholders;

void Placeholders::SetPlaceholders(const std::string& buildMode, const std::string& installPrefix, const std::string& modulePathname)
{
	s_Placeholders["build_mode"] = buildMode;
	std::string lowerBuildMode = buildMode;
	std::transform(lowerBuildMode.begin(), lowerBuildMode.end(), lowerBuildMode.begin(),
		[](unsigned char c) { return std::tolower(c); });
	s_Placeholders["lower_build_mode"] = lowerBuildMode;
	std::string upperBuildMode = buildMode;
	std::transform(upperBuildMode.begin(), upperBuildMode.end(), upperBuildMode.begin(),
		[](unsigned char c) { return std::toupper(c); });
	s_Placeholders["upper_build_mode"] = upperBuildMode;
	s_Placeholders["install_prefix"] = installPrefix;
	s_Placeholders["module_path_name"] = modulePathname;
}

#ifdef WIN32
void Placeholders::SetPlaceholders(const std::string& buildMode, const std::string& installPrefix, const std::string& modulePathname, const std::string& compilerPath)
{
	s_Placeholders["compiler_path"] = compilerPath;
	SetPlaceholders(buildMode, installPrefix, modulePathname);
}
#endif

std::string Placeholders::GetPlaceholder(std::string key)
{
	auto it = s_Placeholders.find(key);
	if (it == s_Placeholders.end())
		return "";
	return s_Placeholders[key];
}
