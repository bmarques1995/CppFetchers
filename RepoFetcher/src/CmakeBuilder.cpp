#include "CmakeBuilder.hpp"
#include "ProcessDispatcher.hpp"
#include <sstream>
#include <iostream>
#include "GitHandler.hpp"
#include <cassert>
#include <algorithm>

#ifdef WIN32
std::string CmakeBuilder::s_SystemName = "windows";
#elif defined(__linux__)
std::string CmakeBuilder::s_SystemName = "linux";
#elif defined(__FreeBSD__)
std::string CmakeBuilder::s_SystemName = "freebsd";
#endif

void CmakeBuilder::GenCmakeSolution(const nlohmann::json& data)
{
	std::string moduleDestination = data["module_destination"].get<std::string>();
	std::vector<std::string> buildArgs;
	GetCmakeGenCommandArgList
	(
		data,
		&buildArgs
	);
	ProcessDispatcher::ExecuteCommand("cmake", buildArgs, moduleDestination);
}

void CmakeBuilder::BuildAndInstallCmakeSolution(const nlohmann::json& data)
{
	std::string moduleDestination;
	std::vector<std::string> buildArgs;
	std::string buildPath = GetCmakeBuildCommandArgList
	(
		data,
		&buildArgs
	);
	ProcessDispatcher::ExecuteCommand("cmake", buildArgs, buildPath);
}

void CmakeBuilder::GetCmakeGenCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs)
{
	std::stringstream commandStream;
	std::stringstream pathBuilder;
	
	assert(data.contains("build_system"));
	assert(data.contains("module_path_name"));
	assert(data.contains("build_mode"));
	assert(data.contains("install_prefix"));
	assert(data.contains("module_destination"));

	std::string modulePathName = data["module_path_name"].get<std::string>();
	std::string buildMode = data["build_mode"].get<std::string>();
	std::string installPrefix = data["install_prefix"].get<std::string>();
	std::string generator = data["build_system"].get<std::string>();
#ifdef WIN32
	std::string compilerPath = "";
	if (data.contains("compiler_path"))
		compilerPath = data["compiler_path"].get<std::string>();
#endif
	std::string relativeRootLocation = "";
	if(!data["relative_root_location"].is_null())
		relativeRootLocation = data["relative_root_location"].get<std::string>();
	std::string moduleDestination = data["module_destination"].get<std::string>();
	std::vector<std::string> flags = std::vector<std::string>();
	if (data.contains("flags"))
	{
		flags = data["flags"].get<std::vector<std::string>>();
	} 
	
	pathBuilder << moduleDestination << "/" << GitHandler::GetModuleInfix() << "/" << modulePathName;
	if (relativeRootLocation.length() > 0)
	{
		pathBuilder << "/" << relativeRootLocation;
	}

#ifdef WIN32
	//if (compilerPath.length() > 0)
		//AppendNinjaVSHost(compilerPath);
#endif
	
	std::string sourcePath = pathBuilder.str();
	buildArgs->push_back("-S");
	buildArgs->push_back(sourcePath);
	pathBuilder.str("");
	
	pathBuilder << moduleDestination << "/dependencies/" << s_SystemName << "/" << modulePathName;
	std::string buildPath = pathBuilder.str();
	pathBuilder.str("");
	buildArgs->push_back("-B");
	buildArgs->push_back(buildPath);
	
	
	if (generator.compare("default") != 0)
	{
		buildArgs->push_back("-G");
		buildArgs->push_back(generator.data());
	}
	
#ifdef WIN32
	commandStream << "-DCMAKE_INSTALL_PREFIX=\"" << installPrefix.data() << "\"";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
	commandStream << "-DCMAKE_PREFIX_PATH=\"" << installPrefix.data() << "\"";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
	commandStream << "-DCMAKE_BUILD_TYPE=\"" << buildMode.data() << "\"";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
#else
	commandStream << "-DCMAKE_INSTALL_PREFIX=" << installPrefix.data() << "";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
	commandStream << "-DCMAKE_PREFIX_PATH=" << installPrefix.data() << "";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
	commandStream << "-DCMAKE_BUILD_TYPE=" << buildMode.data() << "";
	buildArgs->push_back(commandStream.str());
	commandStream.str("");
#endif
	
	
	AppendFlags(buildArgs, flags);
	std::string osFieldPrefix = "os_properties";
	if (!data[osFieldPrefix][s_SystemName].is_null())
	{
		if(!data[osFieldPrefix][s_SystemName]["c_compiler"].is_null())
		{
			buildArgs->push_back("-DCMAKE_C_COMPILER=" + data[osFieldPrefix][s_SystemName]["c_compiler"].get<std::string>());
		}
		if (!data[osFieldPrefix][s_SystemName]["cxx_compiler"].is_null())
		{
			buildArgs->push_back("-DCMAKE_CXX_COMPILER=" + data[osFieldPrefix][s_SystemName]["cxx_compiler"].get<std::string>());
		}
		std::vector<std::string> osFlags = data[osFieldPrefix][s_SystemName].contains("flags") ? data[osFieldPrefix][s_SystemName]["flags"].get<std::vector<std::string>>() : std::vector<std::string>();
		AppendFlags(buildArgs, osFlags);
	}
}

std::string CmakeBuilder::GetCmakeBuildCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs)
{
	std::stringstream pathBuilder;
	assert(!data["module_path_name"].is_null());
	assert(!data["module_destination"].is_null());
	assert(!data["build_system"].is_null());
	assert(!data["build_mode"].is_null());
	buildArgs->push_back("--build");
	std::string buildMode = data["build_mode"].get<std::string>();
	std::string modulePathName = data["module_path_name"].get<std::string>();
	std::string moduleDestination = data["module_destination"].get<std::string>();
	std::string generator = data["build_system"].get<std::string>();
	pathBuilder << moduleDestination << "/dependencies/" << s_SystemName << "/" << modulePathName;
#ifdef WIN32
	std::string buildRawPath = pathBuilder.str();
	std::string buildPath = "\"" + buildRawPath + "\"";
#else
	std::string buildRawPath = pathBuilder.str();
	std::string buildPath = buildRawPath;
#endif
	pathBuilder.str("");
	buildArgs->push_back(buildPath);
#ifdef WIN32
	if(generator.compare("default") == 0)
	{
		buildArgs->push_back("--config");
		buildArgs->push_back(buildMode);
	}
#endif
	buildArgs->push_back("--target");
	buildArgs->push_back("install");
	if (generator.compare("ninja") == 0)
	{
		buildArgs->push_back("--parallel");
	}
	
	return buildRawPath;
}

void CmakeBuilder::AppendFlags(std::vector<std::string>* buildArgs, const std::vector<std::string>& flags)
{
	for (std::string flag : flags)
	{
#ifndef WIN32
		flag.erase(std::remove(flag.begin(), flag.end(), '"'), flag.end());
#endif
		if (flag.starts_with("-D"))
			buildArgs->push_back(flag);
		else
			buildArgs->push_back("-D" + flag);

	}
}
