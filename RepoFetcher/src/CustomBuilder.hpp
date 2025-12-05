#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class CustomBuilder
{
public:
	static void GenSolution(const nlohmann::json& data);
	static void BuildAndInstallSolution(const nlohmann::json& data);
private:
	static void ProcessGenCommandArgList(const std::vector<std::string>& inBuildArgs, std::vector<std::string>* outBuildArgs, std::string* executable);
};