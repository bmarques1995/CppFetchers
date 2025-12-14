#include "MesonBuilder.hpp"
#include "ProcessDispatcher.hpp"
#include "Placeholders.hpp"
#include "FileHandler.hpp"
#include "Utils.hpp"
#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#endif

std::string MesonBuilder::s_PythonName = "";

void MesonBuilder::FindPython()
{
#ifdef WIN32
	FindPythonOnWindows();
#else
	FindPythonOnUnix();
#endif
}

void MesonBuilder::EnableVenv()
{
#ifdef WIN32
	EnableVenvOnWindows();
#else
	EnableVenvOnUnix();
#endif
}

void MesonBuilder::ApplyVenv()
{
#ifdef WIN32
	ApplyVenvOnWindows();
#else
	ApplyVenvOnUnix();
#endif
}

void MesonBuilder::InstallMeson()
{
	ProcessDispatcher::ExecuteCommand("python", { "-m", "pip", "install", "--upgrade", "pip"});
	ProcessDispatcher::ExecuteCommand("python", {"-m", "pip", "install", "meson"});
}

void MesonBuilder::GenSolution(const nlohmann::json& data)
{
	std::unordered_map<std::string, std::string> binariesList;

	AppendIniBinaries(data, &binariesList);
	WriteIniConfigs(binariesList);

	std::vector<std::string> buildArgs;
	GetMesonGenCommandArgList(data, &buildArgs);
	ProcessDispatcher::ExecuteCommand("meson", buildArgs, Placeholders::GetPlaceholder("modules_root"));
	FileHandler::DeleteFileAt(Placeholders::GetPlaceholder("module_root") + "/meson.ini");
}

void MesonBuilder::BuildAndInstallSolution(const nlohmann::json& data)
{
	ProcessDispatcher::ExecuteCommand("meson", { "compile", "-C", Placeholders::GetPlaceholder("module_build_path") });
	ProcessDispatcher::ExecuteCommand("meson", { "install", "-C", Placeholders::GetPlaceholder("module_build_path")});
}

void MesonBuilder::TreatMesonInfo(nlohmann::json* info, std::string buildMode, std::string installPrefix, std::string modulePathname)
{
}

void MesonBuilder::GetMesonGenCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs)
{
	/*meson setup C:\build\libpq_build C:\source\libpq^
	--prefix=C:\my\custom\prefix ^
		--buildtype=release ^
		--backend=ninja*/
#ifdef WIN32
	std::string buildDir = Placeholders::GetPlaceholder("modules_root") + "\\dependencies\\" + Utils::s_SystemName + "\\" + data["git"]["output_suffix"].get<std::string>();
#else
	std::string buildDir = Placeholders::GetPlaceholder("modules_root") + "/dependencies/" + Utils::s_SystemName + "/" + data["git"]["output_suffix"].get<std::string>();
#endif
	Placeholders::SetPlaceholder("module_build_path", buildDir);
	buildArgs->push_back("setup");
	buildArgs->push_back(Placeholders::GetPlaceholder("module_path"));
	buildArgs->push_back(buildDir);
	buildArgs->push_back("--prefix=" + Placeholders::GetPlaceholder("install_prefix"));
	buildArgs->push_back("--buildtype=" + Placeholders::GetPlaceholder("lower_build_mode"));
	buildArgs->push_back("--backend=ninja");
	buildArgs->push_back("--native-file=meson.ini");
}

std::string MesonBuilder::GetMesonBuildCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs)
{
	return std::string();
}

void MesonBuilder::AppendFlags(std::vector<std::string>* buildArgs, const std::vector<std::string>& flags)
{
}

void MesonBuilder::AppendIniBinaries(const nlohmann::json& data, std::unordered_map<std::string, std::string>* binariesList)
{

	std::string osFieldPrefix = "os_properties";
	if (data["meson"][osFieldPrefix].contains(Utils::s_SystemName))
	{
		if (data["meson"][osFieldPrefix][Utils::s_SystemName].contains("c_compiler"))
		{
			(*binariesList)["c"] = data["meson"][osFieldPrefix][Utils::s_SystemName]["c_compiler"].get<std::string>();
		}
		if (data["meson"][osFieldPrefix][Utils::s_SystemName].contains("cxx_compiler"))
		{
			(*binariesList)["cpp"] = data["meson"][osFieldPrefix][Utils::s_SystemName]["cxx_compiler"].get<std::string>();
		}
	}
}

void MesonBuilder::WriteIniConfigs(const std::unordered_map<std::string, std::string>& binariesList)
{
	std::stringstream iniContent;
	if (binariesList.size() > 0)
	{
		iniContent << "[binaries]\n\n";
		for (auto& binary : binariesList)
		{
			iniContent << binary.first << " = " << '\'' << binary.second << '\'' << "\n";
		}
		iniContent << "\n";
	}
#ifdef WIN32
	FileHandler::WriteTextFile(Placeholders::GetPlaceholder("modules_root") + "\\meson.ini", iniContent.str());
#else
	FileHandler::WriteTextFile(Placeholders::GetPlaceholder("modules_root") + "/meson.ini", iniContent.str());
#endif
}

#ifdef WIN32

void MesonBuilder::FindPythonOnWindows()
{
	std::string pyLocation = ProcessDispatcher::SearchExecutableLocation("py");
	if(pyLocation.compare("") == 0)
	{
		std::string pythonLocation = ProcessDispatcher::SearchExecutableLocation("python");
		if(pythonLocation.compare("") == 0)
		{
			std::cerr << "Failed to find Python executable or python manager." << std::endl;
			exit(65);
		}
		s_PythonName = pythonLocation;
	}
	else {
		s_PythonName = pyLocation;
	}
}

void MesonBuilder::EnableVenvOnWindows()
{
	std::string runPath = Placeholders::GetPlaceholder("modules_root");
	std::replace(runPath.begin(), runPath.end(), '/', '\\');
	ProcessDispatcher::ExecuteCommand(s_PythonName, {"-m", "venv", ".win_venv"}, runPath);
}

void MesonBuilder::ApplyVenvOnWindows()
{
	std::unordered_map<std::string, std::string> venvVars;
	std::string venvPath = Placeholders::GetPlaceholder("modules_root");
	venvPath += "\\.win_venv\\Scripts";
	std::replace(venvPath.begin(), venvPath.end(), '/', '\\');
	std::string output;
	ProcessDispatcher::AppendDirectoryToPath(venvPath);
}

#else

void MesonBuilder::FindPythonOnUnix()
{
	std::string pythonLocation = ProcessDispatcher::SearchExecutableLocation("python3");
	if(pythonLocation.compare("") == 0)
	{
		std::string pythonXLocation = ProcessDispatcher::SearchExecutableLocation("python");
		if(pythonXLocation.compare("") == 0)
		{
			std::cerr << "Failed to find any Python executable." << std::endl;
			exit(65);
		}
		s_PythonName = pythonXLocation;
	}
	else {
		s_PythonName = pythonLocation;
	}
}

void MesonBuilder::EnableVenvOnUnix()
{
	std::string runPath = Placeholders::GetPlaceholder("modules_root");
	ProcessDispatcher::ExecuteCommand(s_PythonName, { "-m", "venv", ".unix_venv" }, runPath);
}

void MesonBuilder::ApplyVenvOnUnix()
{
	std::unordered_map<std::string, std::string> venvVars;
	std::string venvPath = Placeholders::GetPlaceholder("modules_root");
	venvPath += "/.unix_venv/bin";
	ProcessDispatcher::AppendDirectoryToPath(venvPath);
}

#endif
