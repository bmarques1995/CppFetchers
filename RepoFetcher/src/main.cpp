// RepoFetcher.cpp: define o ponto de entrada para o aplicativo.
//

#include <iostream>
#include <fstream>
#include "ProcessDispatcher.hpp"
#include "CmakeBuilder.hpp"
#include "CustomBuilder.hpp"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include "GitHandler.hpp"
#include "FileHandler.hpp"
#include <unordered_map>
#include <functional>
#include "Placeholders.hpp"
#include "Utils.hpp"
#include <fmt/core.h>

const std::unordered_map<std::string, std::function<void(nlohmann::json*)>> s_BuildVendors =
{ 
	{
		".cmake", [](nlohmann::json* info)
		{
			std::string buildMode = Placeholders::GetPlaceholder("build_mode");
			std::string installPrefix = Placeholders::GetPlaceholder("install_prefix");
			std::string modulePathname = Placeholders::GetPlaceholder("modules_root");
			CmakeBuilder::TreatCmakeInfo(info, buildMode, installPrefix, modulePathname);
			auto cmakeInfo = (*info)["cmake"].get<nlohmann::json>();
			CmakeBuilder::GenCmakeSolution(cmakeInfo);
			CmakeBuilder::BuildAndInstallCmakeSolution(cmakeInfo);
		}
	},
	{
		".custom", [](nlohmann::json* info)
		{
			CustomBuilder::GenSolution(*info);
			CustomBuilder::BuildAndInstallSolution(*info);
		}
	}
	
};

const std::unordered_map<std::string, std::function<bool(const nlohmann::json&)>> s_BuildVendorsLocated =
{
	{
		".cmake", [](const nlohmann::json&) -> bool
		{
			std::string program = "cmake";
			return ProcessDispatcher::SearchExecutableLocation(program);
		}
	},
	{
		".custom", [](const nlohmann::json& info) -> bool
		{
			std::string genProgram = info["command"]["gen_system"];
			if(info["command"]["os_properties"][Utils::s_SystemName].contains("gen_system"))
				genProgram = info["command"]["os_properties"][Utils::s_SystemName]["gen_system"];

			std::string buildProgram = info["command"]["build_program"];
			if(info["command"]["os_properties"][Utils::s_SystemName].contains("build_program"))
				buildProgram = info["command"]["os_properties"][Utils::s_SystemName]["build_program"];
			
			std::string installProgram = info["command"]["install_program"];
			if (info["command"]["os_properties"][Utils::s_SystemName].contains("install_program"))
				installProgram = info["command"]["os_properties"][Utils::s_SystemName]["install_program"];

			return ProcessDispatcher::SearchExecutableLocation(genProgram) && 
				ProcessDispatcher::SearchExecutableLocation(buildProgram) && 
				ProcessDispatcher::SearchExecutableLocation(installProgram);
		}
	}
};

int main(int argc, char** argv)
{
	//1, 3, 4 e 5
	if (argc < 5)
	{
		std::cout << "Usage: " << argv[0] << " <json_file> <build_mode> <install_prefix> <module_destination> [<compiler_path>]\n";
	}

#ifdef WIN32
	if (argc > 5)
		Placeholders::SetPlaceholders(argv[2], argv[3], argv[4], argv[5]);
	else
		Placeholders::SetPlaceholders(argv[2], argv[3], argv[4]);
#else
	Placeholders::SetPlaceholders(argv[2], argv[3], argv[4]);
#endif // 

#ifdef WIN32
	ProcessDispatcher::ApplyVSEnvironment();
#endif

	std::filesystem::path jsonInput = Utils::GetAbsoluteLocation(argv[1]);
	if (!FileHandler::FileExists(jsonInput.string()))
	{
		std::cout << "File " << argv[1] << " does not exist\n";
		std::exit(65);
	}
	std::string vendor = jsonInput.filename().stem().extension().string();

	std::string jsonRepo;
	FileHandler::ReadTextFile(Utils::GetAbsoluteLocation(argv[1]), &jsonRepo);
	std::ifstream f(jsonRepo);
	nlohmann::json data;
	try
	{ 
		data = nlohmann::json::parse(jsonRepo);
	}
	catch (nlohmann::json::parse_error& e)
	{
		std::cout << "Invalid json file: " << e.what() << "\n";
		std::exit(65);
	}

	auto vendorTestIt = s_BuildVendorsLocated.find(vendor);
	if (vendorTestIt == s_BuildVendorsLocated.end())
	{
		std::cout << "Vendor " << vendor << " not supported\n";
		std::exit(65);
	}

	bool gitFound = ProcessDispatcher::SearchExecutableLocation("git");
	bool vendorFound = vendorTestIt->second(data);

	if (!(gitFound && vendorFound))
	{
		std::cout << "git and vendor must be installed\n";
		std::exit(65);
	}

	auto vendorIt = s_BuildVendors.find(vendor);
	if (vendorIt == s_BuildVendors.end())
	{
		std::cerr << "Vendor " << vendor << " not supported\n";
		std::exit(65);
	}

	
	
	std::string currentPath = std::filesystem::current_path().string();
	ProcessDispatcher::SetExecutableLocation(currentPath);
	GitHandler::ExecuteGitBatch(data, argv[4]);
	vendorIt->second(&data);

	return 0;
}

