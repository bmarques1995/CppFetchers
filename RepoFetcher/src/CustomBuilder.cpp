#include "CustomBuilder.hpp"
#include "Utils.hpp"
#include "ProcessDispatcher.hpp"
#include "Placeholders.hpp"
#include "FileHandler.hpp"

void CustomBuilder::GenSolution(const nlohmann::json& data)
{
	bool usesOSProperties = data["command"]["os_properties"].contains(Utils::s_SystemName);
	std::vector<std::string> genArgs = data["command"]["gen_options"].get<std::vector<std::string>>();
	if (usesOSProperties && data["command"]["os_properties"][Utils::s_SystemName].contains("gen_options"))
		genArgs = data["command"]["os_properties"][Utils::s_SystemName]["gen_options"].get<std::vector<std::string>>();

	std::vector<std::string> finalGenArgs = std::vector<std::string>();
	std::string executable = "";
	ProcessGenCommandArgList(genArgs, &finalGenArgs, &executable);
	std::string shell;
	if (executable.compare("bash") == 0)
	{
		shell = Utils::SaveShellCommand(finalGenArgs[1], "gen");
#ifdef WIN32
		finalGenArgs[1] = Utils::EscapeChars(Utils::WindowsPathToMsys(shell));
#else
		finalGenArgs[1] = shell;
#endif
	}
	//ProcessDispatcher::ExecuteCommand(executable, finalGenArgs, Placeholders::GetPlaceholder("module_path"));
	if (executable.compare("bash") == 0)
	{
		FileHandler::DeleteFile(shell);
	}
}

void CustomBuilder::BuildAndInstallSolution(const nlohmann::json& data)
{
	bool usesOSProperties = data["command"]["os_properties"].contains(Utils::s_SystemName);
	std::vector<std::string> genArgs = data["command"]["build_args"].get<std::vector<std::string>>();
	if (usesOSProperties && data["command"]["os_properties"][Utils::s_SystemName].contains("build_args"))
		genArgs = data["command"]["os_properties"][Utils::s_SystemName]["build_args"].get<std::vector<std::string>>();

	std::vector<std::string> finalBuildArgs = std::vector<std::string>();
	std::string buildExecutable = "";
	ProcessGenCommandArgList(genArgs, &finalBuildArgs, &buildExecutable);
	std::string shell;
	if (buildExecutable.compare("bash") == 0)
	{
		shell = Utils::SaveShellCommand(finalBuildArgs[1], "build");
#ifdef WIN32
		finalBuildArgs[1] = Utils::EscapeChars(Utils::WindowsPathToMsys(shell));
#else
		finalBuildArgs[1] = shell;
#endif
	}
	//ProcessDispatcher::ExecuteCommand(buildExecutable, finalBuildArgs, Placeholders::GetPlaceholder("module_path"));
	if (buildExecutable.compare("bash") == 0)
	{
		FileHandler::DeleteFile(shell);
	}

	std::vector<std::string> installArgs = data["command"]["install_args"].get<std::vector<std::string>>();
	if (usesOSProperties && data["command"]["os_properties"][Utils::s_SystemName].contains("install_args"))
		installArgs = data["command"]["os_properties"][Utils::s_SystemName]["install_args"].get<std::vector<std::string>>();

	std::vector<std::string> finalInstallArgs = std::vector<std::string>();
	std::string installProgram = "";
	ProcessGenCommandArgList(installArgs, &finalInstallArgs, &installProgram);
	if (installProgram.compare("bash") == 0)
	{
		shell = Utils::SaveShellCommand(finalInstallArgs[1], "install");
#ifdef WIN32
		finalInstallArgs[1] = Utils::EscapeChars(Utils::WindowsPathToMsys(shell));
#else
		finalInstallArgs[1] = shell;
#endif
	}
	ProcessDispatcher::ExecuteCommand(installProgram, finalInstallArgs, Placeholders::GetPlaceholder("module_path"));
	if (installProgram.compare("bash") == 0)
	{
		FileHandler::DeleteFile(shell);
	}
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
