#include "CustomBuilder.hpp"
#include "Utils.hpp"
#include "ProcessDispatcher.hpp"
#include "Placeholders.hpp"
#include "FileHandler.hpp"

void CustomBuilder::GenSolution(const nlohmann::json& data)
{
	ExecuteCommand(data, "gen_options");
}

void CustomBuilder::BuildAndInstallSolution(const nlohmann::json& data)
{
	ExecuteCommand(data, "build_args");
	ExecuteCommand(data, "install_args");
}

void CustomBuilder::ProcessGenCommandArgList(const std::vector<std::string>& inBuildArgs, std::vector<std::string>* outBuildArgs, std::string* executable)
{
	auto it = inBuildArgs.begin();
	*executable = *it;
	it++;
	for(; it != inBuildArgs.end(); it++)
	{
		std::string flag = Utils::ProcessFlag(*it);
		if(flag.compare("") != 0)
			outBuildArgs->push_back(flag);
	}	
}

void CustomBuilder::ExecuteCommand(const nlohmann::json& data, std::string arg_list_field)
{
	bool usesOSProperties = data["command"]["os_properties"].contains(Utils::s_SystemName);
	std::vector<std::string> installArgs = data["command"][arg_list_field].get<std::vector<std::string>>();
	if (usesOSProperties && data["command"]["os_properties"][Utils::s_SystemName].contains(arg_list_field))
		installArgs = data["command"]["os_properties"][Utils::s_SystemName][arg_list_field].get<std::vector<std::string>>();

	std::vector<std::string> finalInstallArgs = std::vector<std::string>();
	std::string installProgram = "";
	ProcessGenCommandArgList(installArgs, &finalInstallArgs, &installProgram);
	std::string shell;
	if (installProgram.compare("bash") == 0)
	{
		
#ifdef WIN32
		shell = Utils::SaveShellCommand(finalInstallArgs[1], "install");
		finalInstallArgs[1] = Utils::EscapeChars(Utils::WindowsPathToMsys(shell));
#else
		shell = Utils::SaveShellCommand(finalInstallArgs[0], "install");
		finalInstallArgs[0] = shell;
#endif
	}
	ProcessDispatcher::ExecuteCommand(installProgram, finalInstallArgs, Placeholders::GetPlaceholder("module_path"));
	if (installProgram.compare("bash") == 0)
	{
		FileHandler::DeleteFile(shell);
	}
}
