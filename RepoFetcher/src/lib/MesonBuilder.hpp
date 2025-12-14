#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

//get python name
//python venv
//activate venv
//pip install meson
//meson setup

class MesonBuilder
{
public:
	static void FindPython();
	static void EnableVenv();
	static void ApplyVenv();
	static void InstallMeson();
	static void GenSolution(const nlohmann::json& data);
	static void BuildAndInstallSolution(const nlohmann::json& data);

	static void TreatMesonInfo(nlohmann::json* info, std::string buildMode, std::string installPrefix, std::string modulePathname);

private:
	static void GetMesonGenCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs);
	static std::string GetMesonBuildCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs);

	static void AppendFlags(std::vector<std::string>* buildArgs, const std::vector<std::string>& flags);

	static std::string s_PythonName;

	static void AppendIniBinaries(const nlohmann::json& data, std::unordered_map<std::string, std::string>* binariesList);
	static void WriteIniConfigs(const std::unordered_map<std::string, std::string>& binariesList);

#ifdef WIN32
	static void FindPythonOnWindows();
	static void EnableVenvOnWindows();
	static void ApplyVenvOnWindows();
#else
	static void FindPythonOnUnix();
	static void EnableVenvOnUnix();
	static void ApplyVenvOnUnix();
#endif
};