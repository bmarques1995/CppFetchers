#include "GitHandler.hpp"
#include <sstream>
#include "ProcessDispatcher.hpp"
#include "Utils.hpp"
#include "Placeholders.hpp"

std::string GitHandler::s_ModuleInfix = "modules";
std::string GitHandler::s_PatchesRelativePath = "patches";

void GitHandler::ExecuteGitBatch(const nlohmann::json& data, const std::string& moduleDestination)
{
	std::string outputSuffix = data["git"]["output_suffix"].get<std::string>();
	std::string repoLocation = data["git"]["location"].get<std::string>();
	GitHandler::CloneRepository(repoLocation, outputSuffix, Utils::GetAbsoluteLocation(moduleDestination));
	std::stringstream outputRepoDirStream;
	outputRepoDirStream << moduleDestination << "/" << GitHandler::GetModuleInfix() << "/" << outputSuffix;
	std::string outputRepoDir = Utils::GetAbsoluteLocation(outputRepoDirStream.str());
	Placeholders::SetPlaceholder("module_path", outputRepoDir);
#ifdef WIN32
	Placeholders::SetPlaceholder("msys_escaped_module_path", Utils::EscapeChars(Utils::WindowsPathToMsys(outputRepoDir)));
#endif
	outputRepoDirStream.str("");
	if (!(data["git"]["commit"].is_null()))
	{
		std::string commitHash = data["git"]["commit"].get<std::string>();
		if (commitHash.compare("") != 0)
		{
			GitHandler::Rollback(outputRepoDir, commitHash);
		}
	}
	std::string patch = data["git"].contains("patch") ? data["git"]["patch"].get<std::string>() : "";
	if (patch.compare("") != 0)
	{
		outputRepoDirStream << moduleDestination << "/" << GitHandler::GetPatchesRelativePath() << "/" << data["git"]["patch"].get<std::string>();
		std::string patchDir = Utils::GetAbsoluteLocation(outputRepoDirStream.str());
		outputRepoDirStream.str("");
		GitHandler::ApplyPatch(outputRepoDir, patchDir);
	}
}
bool GitHandler::CloneRepository(std::string_view url, std::string_view repoSuffix, std::string_view projectDirectory, std::string_view branch)
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
    return ProcessDispatcher::ExecuteCommand("git", args, ProcessDispatcher::GetExecutableLocation());
}

bool GitHandler::ApplyPatch(std::string_view directory, std::string_view patchDirectory)
{
	std::vector<std::string> args;
	args.push_back("apply");
	args.push_back("-v");
	std::stringstream patchStream;
#ifdef WIN32
	patchStream << "\"" << patchDirectory.data() << "\"";
#else
	patchStream << patchDirectory.data();
#endif
	args.push_back(patchStream.str());
	return ProcessDispatcher::ExecuteCommand("git", args, directory.data());
}

bool GitHandler::Rollback(std::string_view directory, std::string_view commitHash)
{
	std::vector<std::string> args;
	args.push_back("reset");
	args.push_back("--hard");
	args.push_back(commitHash.data());
	return ProcessDispatcher::ExecuteCommand("git", args, directory.data());
}

void GitHandler::SetPatchesRelativePath(std::string_view path)
{
	s_PatchesRelativePath = path;
}

const std::string& GitHandler::GetPatchesRelativePath()
{
	// TODO: inserir instrução return aqui
	return s_PatchesRelativePath;
}

void GitHandler::SetModuleInfix(std::string_view infix)
{
    s_ModuleInfix = infix;
}

const std::string& GitHandler::GetModuleInfix()
{
	return s_ModuleInfix;
}
