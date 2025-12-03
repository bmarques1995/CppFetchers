#include "CustomBuilder.hpp"
#include "Utils.hpp"
#include "ProcessDispatcher.hpp"
#include "Placeholders.hpp"

void CustomBuilder::GenSolution(const nlohmann::json& data)
{
	std::string genProgram = data["command"]["gen_system"];
	if (data["command"]["os_properties"][Utils::s_SystemName].contains("gen_system"))
		genProgram = data["command"]["os_properties"][Utils::s_SystemName]["gen_system"];

	std::vector<std::string> genArgs = data["command"]["gen_options"].get<std::vector<std::string>>();
	if (data["command"]["os_properties"][Utils::s_SystemName].contains("gen_options"))
		genArgs = data["command"]["os_properties"][Utils::s_SystemName]["gen_options"].get<std::vector<std::string>>();

	std::vector<std::string> finalGenArgs = std::vector<std::string>();
	ProcessGenCommandArgList(genArgs, &finalGenArgs);
	ProcessDispatcher::ExecuteCommand(genProgram, finalGenArgs, Placeholders::GetPlaceholder("module_path"));
}

void CustomBuilder::BuildAndInstallSolution(const nlohmann::json& data)
{
	std::string buildProgram = data["command"]["build_program"];
	if (data["command"]["os_properties"][Utils::s_SystemName].contains("build_program"))
		buildProgram = data["command"]["os_properties"][Utils::s_SystemName]["build_program"];

	std::vector<std::string> genArgs = data["command"]["build_args"].get<std::vector<std::string>>();
	if (data["command"]["os_properties"][Utils::s_SystemName].contains("build_args"))
		genArgs = data["command"]["os_properties"][Utils::s_SystemName]["build_args"].get<std::vector<std::string>>();

	std::vector<std::string> finalBuildArgs = std::vector<std::string>();
	ProcessGenCommandArgList(genArgs, &finalBuildArgs);
	ProcessDispatcher::ExecuteCommand(buildProgram, finalBuildArgs, Placeholders::GetPlaceholder("module_path"));

	std::string installProgram = data["command"]["build_program"];
	if (data["command"]["os_properties"][Utils::s_SystemName].contains("build_program"))
		installProgram = data["command"]["os_properties"][Utils::s_SystemName]["build_program"];

	std::vector<std::string> installArgs = data["command"]["install_args"].get<std::vector<std::string>>();
	if (data["command"]["os_properties"][Utils::s_SystemName].contains("build_args"))
		installArgs = data["command"]["os_properties"][Utils::s_SystemName]["build_args"].get<std::vector<std::string>>();

	std::vector<std::string> finalInstallArgs = std::vector<std::string>();
	ProcessGenCommandArgList(installArgs, &finalInstallArgs);
	ProcessDispatcher::ExecuteCommand(installProgram, finalInstallArgs, Placeholders::GetPlaceholder("module_path"));
	
}

void CustomBuilder::ProcessGenCommandArgList(const std::vector<std::string>& inBuildArgs, std::vector<std::string>* outBuildArgs)
{
	for(auto it = inBuildArgs.begin(); it != inBuildArgs.end(); it++)
	{
		outBuildArgs->push_back(Utils::ProcessFlag(*it));
	}	
}
