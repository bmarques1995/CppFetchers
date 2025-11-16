#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class CmakeBuilder
{
public:
	static void GenCmakeSolution(const nlohmann::json& data);
	static void BuildAndInstallCmakeSolution(const nlohmann::json& data);

private:
	static void GetCmakeGenCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs);
	static std::string GetCmakeBuildCommandArgList(const nlohmann::json& data, std::vector<std::string>* buildArgs);

	static void AppendFlags(std::vector<std::string>* buildArgs, const std::vector<std::string>& flags);

	static std::string s_SystemName;
};
