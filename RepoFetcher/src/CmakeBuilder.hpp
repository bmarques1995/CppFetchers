#pragma once

#include <string>
#include <vector>

class CmakeBuilder
{
public:
	static void BuildCmakeProject
	(
		std::string_view modulePathName,
		std::string_view moduleDestination,
		std::string_view buildMode,
		std::string_view installPrefix,
		std::string_view generator,
		const std::vector<std::string>& flags,
		std::string_view compilerPath = ""
	);

private:
	static void GetCmakeGenCommandArgList
	(
		std::string_view modulePathName,
		std::string_view moduleDestination,
		std::string_view buildMode,
		std::string_view installPrefix,
		std::string_view generator,
		const std::vector<std::string>& flags,
		std::string_view compilerPath,
		std::vector<std::string>* buildArgs
	);
};
