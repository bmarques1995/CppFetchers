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
#include "MesonBuilder.hpp"
#include "Utils.hpp"
#include <fmt/core.h>

inline const std::unordered_map<std::string, std::function<void(nlohmann::json*)>> s_BuildVendors =
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
		".meson", [](nlohmann::json* info)
		{
			MesonBuilder::GenSolution(*info);
			MesonBuilder::BuildAndInstallSolution(*info);
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

inline const std::unordered_map<std::string, std::function<bool(const nlohmann::json&)>> s_BuildVendorsLocated =
{
	{
		".cmake", [](const nlohmann::json&) -> bool
		{
			std::string program = "cmake";
			return (ProcessDispatcher::SearchExecutableLocation(program).compare("") != 0);
		}
	},
	{
		".meson", [](const nlohmann::json& info) -> bool
		{
			MesonBuilder::FindPython();
			MesonBuilder::EnableVenv();
			MesonBuilder::ApplyVenv();
			ProcessDispatcher::AppendDirectoryToPath(Placeholders::GetPlaceholder("install_prefix"));
#ifndef WIN32
			std::string libPath = Placeholders::GetPlaceholder("install_prefix") + "/lib";
			std::string binPath = Placeholders::GetPlaceholder("install_prefix") + "/bin";
			ProcessDispatcher::AppendDirectoryToPath(libPath);
			ProcessDispatcher::AppendDirectoryToPath(binPath);
			ProcessDispatcher::AppendVariable("LD_LIBRARY_PATH", libPath);
#endif
			MesonBuilder::InstallMeson();
			return true;
		}
	},
	{
		".custom", [](const nlohmann::json& info) -> bool
		{
			bool usesOSProperties = info["command"]["os_properties"].contains(Utils::s_SystemName);
			std::vector<std::string> testPrograms = info["command"]["test_programs"];
			if(usesOSProperties && info["command"]["os_properties"][Utils::s_SystemName].contains("test_programs"))
				testPrograms = info["command"]["os_properties"][Utils::s_SystemName]["test_programs"];
			
			bool programsFound = true;
			for (std::string program : testPrograms)
			{
				programsFound = programsFound && (ProcessDispatcher::SearchExecutableLocation(program).compare("") != 0);
				if (!programsFound)
				{
					std::cerr << "Program " << program << " not found\n";
					exit(65);
				}
			}
			return programsFound;
		}
	}
};

int main(int argc, char** argv)
{
	//1, 3, 4 e 5
	if (argc < 5)
	{
		std::cerr << "Usage: " << argv[0] << " <json_file> <build_mode> <install_prefix> <module_destination> [<compiler_path>]\n";
		std::exit(65);
	}

	std::string jsonfileFullpath = Utils::ExpandPath(argv[1]);
	std::string installPrefixFullpath = Utils::ExpandPath(argv[3]);
	std::string moduleDestinationFullpath = Utils::ExpandPath(argv[4]);

	// 1, 3, 4 e 5

#ifdef WIN32
	if (argc > 5)
	{
		std::string msvcCompilerFullpath = Utils::ExpandPath(argv[5]);
		Placeholders::SetPlaceholders(argv[2], installPrefixFullpath, moduleDestinationFullpath, msvcCompilerFullpath);
	}
	else
		Placeholders::SetPlaceholders(argv[2], installPrefixFullpath, moduleDestinationFullpath);
#else
	Placeholders::SetPlaceholders(argv[2], installPrefixFullpath, moduleDestinationFullpath);
#endif // 

#ifdef WIN32
	ProcessDispatcher::ApplyVSEnvironment();
#endif

	ProcessDispatcher::FilterPath();

	std::filesystem::path jsonInput = Utils::GetAbsoluteLocation(argv[1]);
	if (!FileHandler::FileExists(jsonInput.string()))
	{
		std::cerr << "File " << argv[1] << " does not exist\n";
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
		std::cerr << "Invalid json file: " << e.what() << "\n";
		std::exit(65);
	}

	auto vendorTestIt = s_BuildVendorsLocated.find(vendor);
	if (vendorTestIt == s_BuildVendorsLocated.end())
	{
		
		std::cerr << "\nVendor " << vendor << " not supported\n";
		std::exit(65);
	}

	bool gitFound = (ProcessDispatcher::SearchExecutableLocation("git").compare("") != 0);
	bool vendorsFound = vendorTestIt->second(data);

	if (!(gitFound && vendorsFound))
	{
		std::cerr << "git and vendor must be installed\n";
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

