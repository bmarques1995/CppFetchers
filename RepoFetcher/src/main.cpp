// RepoFetcher.cpp: define o ponto de entrada para o aplicativo.
//

#include <iostream>
#include "ProcessDispatcher.hpp"
#include "CmakeBuilder.hpp"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include "GitCloner.hpp"

int main(int argc, char** argv)
{
	/*if (argc < 5)
	{
		std::cout << "Usage: " << argv[0] << " <json_file> <build_mode> <install_prefix> <module_destination> [<compiler_path>]\n";
	}*/

	bool gitFound = ProcessDispatcher::SearchExecutableLocation("git");
	bool cmakeFound = ProcessDispatcher::SearchExecutableLocation("cmake");

	if(!(gitFound && cmakeFound))
	{
		std::cout << "git and cmake must be installed\n";
		std::exit(65);
	}

	const std::string jsonRepo = R"({
"git": {
"location": "https://github.com/libsdl-org/SDL.git",
"output_suffix": "sdl3",
"branch": "default",
"commit": "latest",
"patch": "drop"

},
"cmake": {
"build_system": "default",
"flags": ["-DBUILD_SHARED_LIBS=ON"]
}
})";


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
		std::string outputSuffix = data["git"]["output_suffix"];
#ifdef WIN32
		GitCloner::CloneRepository("https://github.com/libsdl-org/SDL.git", outputSuffix, "D:\\cpp\\CppFetchers");
#elif defined(__linux__)
		GitCloner::CloneRepository("https://github.com/libsdl-org/SDL.git", outputSuffix, "/mnt/d/cpp/CppFetchers");
#endif
		//ProcessDispatcher::ExecuteCommand("git", {"clone", "https://github.com/libsdl-org/SDL.git"});
	}
	if (cmakeFound)
	{
#ifdef WIN32
		CmakeBuilder::BuildCmakeProject("sdl3", "D:\\cpp\\CppFetchers", "Debug", "D:\\cpp\\CppFetchers\\Windows\\Debug", "default", data["cmake"]["flags"]);
#elif defined(__linux__)
		CmakeBuilder::BuildCmakeProject("sdl3", "/mnt/d/cpp/CppFetchers", "Debug", "/mnt/d/cpp/CppFetchers/Linux/Debug", "default", data["cmake"]["flags"]);
#endif
	}

	return 0;
}
