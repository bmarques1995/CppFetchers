#include "GitCloner.hpp"
#include <sstream>
#include "ProcessDispatcher.hpp"

std::string GitCloner::s_ModuleInfix = "modules";
bool GitCloner::CloneRepository(std::string_view url, std::string_view repoSuffix, std::string_view projectDirectory, std::string_view branch)
{
    std::vector<std::string> args;
	args.push_back("clone");
	args.push_back("--recursive");
	if (branch.length() > 0)
	{
		args.push_back("--branch");
		args.push_back(branch.data());
	}
	args.push_back(url.data());
	std::string rawDirectory = projectDirectory.data();
	/*Mover para classe Sanitizers*/
	if (rawDirectory[rawDirectory.length() - 1] == '/' || rawDirectory[rawDirectory.length() - 1] == '\\')
	{
		rawDirectory = rawDirectory.substr(0, rawDirectory.length() - 1);
	}
	std::stringstream dirStream;
	dirStream << projectDirectory.data() << "/" << s_ModuleInfix << "/" << repoSuffix.data();
	std::string directory = dirStream.str();
	args.push_back(directory);
	ProcessDispatcher::ExecuteCommand("git", args, ProcessDispatcher::GetExecutableLocation());
    return false;
}

bool GitCloner::ApplyPatch(std::string_view directory, std::string_view patchDirectory)
{
    return false;
}

bool GitCloner::Rollback(std::string_view directory, std::string_view commitHash)
{
    return false;
}

void GitCloner::SetModuleInfix(std::string_view infix)
{
    s_ModuleInfix = infix;
}

const std::string& GitCloner::GetModuleInfix()
{
	return s_ModuleInfix;
}
