// RepoFetcher.cpp: define o ponto de entrada para o aplicativo.
//

#include <iostream>
#include "ProcessDispatcher.hpp"
#include <nlohmann/json.hpp>
#include <cstdlib>

int main(int argc, char** argv)
{
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
"output_suffix": "<name_output_dir>",
"branch": "default|<branch_name>",
"commit": "latest|<commit_hash>",
"patch": "drop|<patch_file>"

},
"cmake": {
"build_system": "default|<ninja|\"cmake --help\">",
"flags": ["-DVARIABLE=VALUE", "-DVARIABLE2=VALUE2"]
}
})";


	nlohmann::json data = nlohmann::json::parse(jsonRepo);
	
	std::cout << data["git"]["location"] << "\n";
	// std::cout << (data["cmake"]["build_sistem"].is_null() ? "default" : data["cmake"]["build_sistem"]) << "\n";
	std::vector<const char*> type;
	auto data_type = type.data();
	for (auto it = data["cmake"]["flags"].begin(); it < data["cmake"]["flags"].end(); it++)
	{
		std::cout << *it << "\n";
	}
	if (gitFound)
	{
		ProcessDispatcher::ExecuteCommand("git", {"clone", "https://github.com/libsdl-org/SDL.git"});
	}
	return 0;
}
