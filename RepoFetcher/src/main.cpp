// RepoFetcher.cpp: define o ponto de entrada para o aplicativo.
//

#include <iostream>
#include <fstream>
#include "ProcessDispatcher.hpp"
#include "CmakeBuilder.hpp"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include "GitHandler.hpp"
#include "FileHandler.hpp"
#include <unordered_map>
#include <functional>
#include "Placeholders.hpp"
#include "Utils.hpp"

const std::unordered_map<std::string, std::function<void(nlohmann::json*)>> s_BuildVendors =
{ 
	{
		".cmake", [](nlohmann::json* info)
		{
			std::string buildMode = Placeholders::GetPlaceholder("build_mode");
			std::string installPrefix = Placeholders::GetPlaceholder("install_prefix");
			std::string modulePathname = Placeholders::GetPlaceholder("module_path_name");
#ifdef WIN32
			std::string compilerPath = Placeholders::GetPlaceholder("compiler_path");
			if (!compilerPath.empty())
			{
				CmakeBuilder::TreatCmakeInfo(info, buildMode, installPrefix, modulePathname, compilerPath);
			}
			else
			{
				CmakeBuilder::TreatCmakeInfo(info, buildMode, installPrefix, modulePathname);
			}
#else
			CmakeBuilder::TreatCmakeInfo(info, buildMode, installPrefix, modulePathname);
#endif
			auto cmakeInfo = (*info)["cmake"].get<nlohmann::json>();
			CmakeBuilder::GenCmakeSolution(cmakeInfo);
			CmakeBuilder::BuildAndInstallCmakeSolution(cmakeInfo);
		}
	}
};

const std::unordered_map<std::string, std::function<bool(const std::string&)>> s_BuildVendorsLocated =
{
	{
		".cmake", [](const std::string&) -> bool
		{
			std::string program = "cmake";
			return ProcessDispatcher::SearchExecutableLocation(program);
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

	

	std::filesystem::path jsonInput = Utils::GetAbsoluteLocation(argv[1]);
	if (!FileHandler::FileExists(jsonInput.string()))
	{
		std::cout << "File " << argv[1] << " does not exist\n";
		std::exit(65);
	}
	std::string vendor = jsonInput.filename().stem().extension().string();

	auto vendorTestIt = s_BuildVendorsLocated.find(vendor);
	if (vendorTestIt == s_BuildVendorsLocated.end() || !vendorTestIt->second(vendor))
	{
		std::cout << "Vendor " << vendor << " not supported\n";
		std::exit(65);
	}

	bool gitFound = ProcessDispatcher::SearchExecutableLocation("git");
	bool vendorFound = vendorTestIt->second(vendor);

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

	std::string jsonRepo;
	FileHandler::ReadTextFile(Utils::GetAbsoluteLocation(argv[1]), &jsonRepo);
	std::ifstream f(jsonRepo);
	nlohmann::json data = nlohmann::json::parse(jsonRepo);
	
	std::string currentPath = std::filesystem::current_path().string();
	ProcessDispatcher::SetExecutableLocation(currentPath);
	GitHandler::ExecuteGitBatch(data, argv[4]);
	vendorIt->second(&data);

	return 0;
}

