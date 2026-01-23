#include "CmakeBuilder.hpp"
#include "ProcessDispatcher.hpp"
#include <sstream>
#include <iostream>
#include "GitHandler.hpp"
#include <cassert>
#include <algorithm>
#include "Utils.hpp"

#ifdef WIN32
#include <windows.h>
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

void CmakeBuilder::TreatCmakeInfo(nlohmann::json* info, std::string buildMode, std::string installPrefix, std::string modulePathname)
{
	(*info)["cmake"]["module_destination"] = Utils::GetAbsoluteLocation(modulePathname);
	(*info)["cmake"]["build_mode"] = buildMode;
	(*info)["cmake"]["install_prefix"] = Utils::GetAbsoluteLocation(installPrefix);
	(*info)["cmake"]["module_path_name"] = (*info)["git"]["output_suffix"].get<std::string>();
}


void CmakeBuilder::GetCmakeGenCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs)
{
	std::stringstream commandStream;
	std::stringstream pathBuilder;
	std::string osFieldPrefix = "os_properties";

	assert(data.contains("build_system"));
	assert(data.contains("module_path_name"));
	assert(data.contains("build_mode"));
	assert(data.contains("install_prefix"));
	assert(data.contains("module_destination"));

	std::string modulePathName = data["module_path_name"].get<std::string>();
	std::string buildMode = data["build_mode"].get<std::string>();
	std::string installPrefix = data["install_prefix"].get<std::string>();
	std::string generator = data["build_system"].get<std::string>();
	if (data[osFieldPrefix].contains(Utils::s_SystemName))
	{
		if (data[osFieldPrefix][Utils::s_SystemName].contains("build_system"))
		{
			generator = data[osFieldPrefix][Utils::s_SystemName]["build_system"].get<std::string>();
		}
	}
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
		std::vector<std::string> rawFlags = data["flags"].get<std::vector<std::string>>();
		for (auto it = rawFlags.begin(); it != rawFlags.end(); it++)
		{
			std::string flag = *it;
			flag = Utils::ProcessFlag(flag);
			flags.push_back(flag);
		}
	}
	
	if (data[osFieldPrefix][Utils::s_SystemName].contains("flags"))
	{
		std::vector<std::string> systemFlags = data[osFieldPrefix][Utils::s_SystemName]["flags"].get<std::vector<std::string>>();
		for (auto it = systemFlags.begin(); it != systemFlags.end(); it++)
		{
			std::string flag = *it;
			flag = Utils::ProcessFlag(flag);
			flags.push_back(flag);
		}

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
	
	pathBuilder << moduleDestination << "/dependencies/" << Utils::s_SystemName << "/" << modulePathName;
	std::string buildPath = pathBuilder.str();
	pathBuilder.str("");
	buildArgs->push_back("-B");
	buildArgs->push_back(buildPath);
	
	
	if (generator.compare("default") != 0)
	{
		buildArgs->push_back("-G");
		if (!generator.empty()) {
			generator[0] = std::toupper(generator[0]);
		}
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
	if (data[osFieldPrefix].contains(Utils::s_SystemName))
	{
		if(data[osFieldPrefix][Utils::s_SystemName].contains("c_compiler"))
		{
			buildArgs->push_back("-DCMAKE_C_COMPILER=" + data[osFieldPrefix][Utils::s_SystemName]["c_compiler"].get<std::string>());
		}
		if (data[osFieldPrefix][Utils::s_SystemName].contains("cxx_compiler"))
		{
			buildArgs->push_back("-DCMAKE_CXX_COMPILER=" + data[osFieldPrefix][Utils::s_SystemName]["cxx_compiler"].get<std::string>());
		}
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
	pathBuilder << moduleDestination << "/dependencies/" << Utils::s_SystemName << "/" << modulePathName;
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
