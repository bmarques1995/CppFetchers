#pragma once

#include <string>
#include <vector>

class GitCloner
{
public:
	static bool CloneRepository(std::string_view url, std::string_view repoSuffix, std::string_view projectDirectory, std::string_view branch = "");
	static bool ApplyPatch(std::string_view directory, std::string_view patchDirectory);
	static bool Rollback(std::string_view directory, std::string_view commitHash);

	static void SetModuleInfix(std::string_view infix);
	static const std::string& GetModuleInfix();
private:
	static std::string s_ModuleInfix;
};
