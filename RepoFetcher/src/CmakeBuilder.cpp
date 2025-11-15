#include "CmakeBuilder.hpp"
#include "ProcessDispatcher.hpp"
#include <sstream>
#include <iostream>

void CmakeBuilder::BuildCmakeProject
(
	std::string_view modulePathName,
	std::string_view moduleDestination,
	std::string_view buildMode,
	std::string_view installPrefix,
	std::string_view generator,
	const std::vector<std::string>& flags,
	std::string_view compilerPath
)
{
	std::vector<std::string> buildArgs;
	GetCmakeGenCommandArgList
	(
		modulePathName,
		moduleDestination,
		buildMode,
		installPrefix,
		generator,
		flags,
		compilerPath,
		&buildArgs
	);
	ProcessDispatcher::ExecuteCommand("cmake", buildArgs, moduleDestination.data());
}

void CmakeBuilder::GetCmakeGenCommandArgList
(
	std::string_view modulePathName,
	std::string_view moduleDestination,
	std::string_view buildMode,
	std::string_view installPrefix,
	std::string_view generator,
	const std::vector<std::string>& flags,
	std::string_view compilerPath,
	std::vector<std::string>* buildArgs
)
{
	std::stringstream commandStream;
	std::stringstream pathBuilder;

#ifdef WIN32
	std::string outputPathInfix = "windows";
#elif defined(__linux__)
	std::string outputPathInfix = "linux";
#elif defined(__FreeBSD__)
	std::string outputPathInfix = "freebsd";
#endif // 

	pathBuilder << moduleDestination << "/modules/" << modulePathName;
	std::string sourcePath = pathBuilder.str();
	buildArgs->push_back("-S");
	buildArgs->push_back(sourcePath);
	pathBuilder.str("");

	pathBuilder << moduleDestination << "/dependencies/" << outputPathInfix << "/" << modulePathName;
	std::string buildPath = pathBuilder.str();
	pathBuilder.str("");
	buildArgs->push_back("-B");
	buildArgs->push_back(buildPath);
	

	if (generator.compare("default") != 0)
	{
		buildArgs->push_back("-G");
		buildArgs->push_back(generator.data());
	}

	if(compilerPath.length() > 0)
		std::cout << "Append compiler path\n";

	commandStream << "-DCMAKE_INSTALL_PREFIX=\"" << installPrefix.data() << "\"";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
	commandStream << "-DCMAKE_PREFIX_PATH=\"" << installPrefix.data() << "\"";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
	commandStream << "-DCMAKE_BUILD_TYPE=\"" << buildMode.data() << "\"";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");

	for (std::string flag : flags)
	{
		buildArgs->push_back(flag);
	}
}
