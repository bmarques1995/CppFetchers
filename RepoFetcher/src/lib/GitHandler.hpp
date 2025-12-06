#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class GitHandler
{
public:
	static void ExecuteGitBatch(const nlohmann::json& data, const std::string& moduleDestination);
	static bool CloneRepository(std::string_view url, std::string_view repoSuffix, std::string_view projectDirectory, std::string_view branch = "");
	static bool ApplyPatch(std::string_view directory, std::string_view patchDirectory);
	static bool Rollback(std::string_view directory, std::string_view commitHash);

	static void SetPatchesRelativePath(std::string_view path);
	static const std::string& GetPatchesRelativePath();

	static void SetModuleInfix(std::string_view infix);
	static const std::string& GetModuleInfix();
private:
	static std::string s_ModuleInfix;
	static std::string s_PatchesRelativePath;
};
