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

const std::unordered_map<std::string, std::function<void(const nlohmann::json&)>> s_BuildVendors =
{ 
	{
		".cmake", [](const nlohmann::json& cmakeInfo)
		{
			CmakeBuilder::GenCmakeSolution(cmakeInfo);
			CmakeBuilder::BuildAndInstallCmakeSolution(cmakeInfo);
		}
	}
};



std::string GetAbsoluteLocation(const std::string& path);
int main(int argc, char** argv)
{
	//1, 3, 4 e 5
	if (argc < 5)
	{
		std::cout << "Usage: " << argv[0] << " <json_file> <build_mode> <install_prefix> <module_destination> [<compiler_path>]\n";
	}

	bool gitFound = ProcessDispatcher::SearchExecutableLocation("git");
	bool cmakeFound = ProcessDispatcher::SearchExecutableLocation("cmake");

	if(!(gitFound && cmakeFound))
	{
		std::cout << "git and cmake must be installed\n";
		std::exit(65);
	}

	std::filesystem::path jsonInput = GetAbsoluteLocation(argv[1]);
	if (!FileHandler::FileExists(jsonInput.string()))
	{
		std::cout << "File " << argv[1] << " does not exist\n";
		std::exit(65);
	}
	std::string vendor = jsonInput.filename().stem().extension().string();

	auto vendorIt = s_BuildVendors.find(vendor);
	if (vendorIt == s_BuildVendors.end())
	{
		std::cerr << "Vendor " << vendor << " not supported\n";
		std::exit(65);
	}

	std::string jsonRepo;
	FileHandler::ReadTextFile(GetAbsoluteLocation(argv[1]), &jsonRepo);
	std::ifstream f(jsonRepo);
	nlohmann::json data = nlohmann::json::parse(jsonRepo);
	
	//std::cout << data["git"]["location"] << "\n";
	//// std::cout << (data["cmake"]["build_sistem"].is_null() ? "default" : data["cmake"]["build_sistem"]) << "\n";
	//for (auto it = data["cmake"]["flags"].begin(); it < data["cmake"]["flags"].end(); it++)
	//{
	//	std::cout << *it << "\n";
	//}
	std::string currentPath = std::filesystem::current_path().string();
	ProcessDispatcher::SetExecutableLocation(currentPath);
	if (gitFound)
	{
		std::string outputSuffix = data["git"]["output_suffix"].get<std::string>();;
		std::string repoLocation = data["git"]["location"].get<std::string>();;
		GitHandler::CloneRepository(repoLocation, outputSuffix, GetAbsoluteLocation(argv[4]));
		std::stringstream outputRepoDirStream;
		outputRepoDirStream << argv[4] << "/" << GitHandler::GetModuleInfix() << "/" << outputSuffix;
		std::string outputRepoDir = GetAbsoluteLocation(outputRepoDirStream.str());
		outputRepoDirStream.str("");
		if(!(data["git"]["commit"].is_null()))
		{
			std::string commitHash = data["git"]["commit"].get<std::string>();
			GitHandler::Rollback(outputRepoDir, commitHash);
		}
		std::string patch = data["git"]["patch"].is_null() ? "" : data["git"]["patch"].get<std::string>();
		if (patch.compare("") != 0)
		{
			outputRepoDirStream << argv[4] << "/" << GitHandler::GetPatchesRelativePath() << "/" << data["git"]["patch"].get<std::string>();
			std::string patchDir = GetAbsoluteLocation(outputRepoDirStream.str());
			outputRepoDirStream.str("");
			GitHandler::ApplyPatch(outputRepoDir, patchDir);
		}
	}
	if (cmakeFound)
	{
		auto cmakeInfo = data["cmake"].get<nlohmann::json>();
		cmakeInfo["module_destination"] = GetAbsoluteLocation(argv[4]);
		cmakeInfo["build_mode"] = argv[2];
		cmakeInfo["install_prefix"] = GetAbsoluteLocation(argv[3]);
		cmakeInfo["module_path_name"] = data["git"]["output_suffix"].get<std::string>();
#ifdef WIN32
		if(argc > 5)
			cmakeInfo["compiler_path"] = argv[5];
#endif
		vendorIt->second(cmakeInfo);
	}

	return 0;
}

std::string GetAbsoluteLocation(const std::string& path)
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
