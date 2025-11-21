#include "ProcessDispatcher.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <filesystem>

#ifdef WIN32
#include <windows.h>
#include <regex>
#elif defined(__linux__) || defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

std::string ProcessDispatcher::s_ExecutableLocation = "";

namespace ProcessDispatcherSource
{
#ifdef WIN32
	bool SearchExecutableLocationOnWindows(std::string_view programName);
	bool ExecuteCommandOnWindows(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory);
    void AppendDirectoryToPathOnWindows(std::string_view directory);
#elif defined(__linux__) || defined(__FreeBSD__)
    bool SearchExecutableLocationOnUnix(std::string_view programName);
    bool ExecuteCommandOnUnix(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory);
    void AppendDirectoryToPathOnUnix(std::string_view directory);
#endif // WIN32
}

bool ProcessDispatcher::SearchExecutableLocation(std::string_view programName)
{
#ifdef WIN32
	return ProcessDispatcherSource::SearchExecutableLocationOnWindows(programName);
#elif defined(__linux__) || defined(__FreeBSD__)
    return ProcessDispatcherSource::SearchExecutableLocationOnUnix(programName);
#endif
}

bool ProcessDispatcher::ExecuteCommand(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory)
{
#ifdef WIN32
	return ProcessDispatcherSource::ExecuteCommandOnWindows(command, arguments, workingDirectory);
#elif defined(__linux__) || defined(__FreeBSD__)
    return ProcessDispatcherSource::ExecuteCommandOnUnix(command, arguments, workingDirectory);
#endif
}

void ProcessDispatcher::AppendDirectoryToPath(std::string_view directory)
{
#ifdef WIN32
    return ProcessDispatcherSource::AppendDirectoryToPathOnWindows(directory);
#elif defined(__linux__) || defined(__FreeBSD__)
    return ProcessDispatcherSource::AppendDirectoryToPathOnUnix(directory);
#endif
}

void ProcessDispatcher::SetExecutableLocation(std::string_view location)
{
    s_ExecutableLocation = location;
}

const std::string& ProcessDispatcher::GetExecutableLocation()
{
    return s_ExecutableLocation;
}

#ifdef WIN32

std::string ProcessDispatcher::ExtractVSBasePath(const std::string& path)
{
    std::string copyPath = std::regex_replace(path, std::regex("/"), "\\");
    std::regex re(R"(.*(?:Community|Enterprise|Professional))");
    std::smatch match;

    if (std::regex_search(copyPath, match, re)) {
        std::string editionPath = match[0].str();
        std::replace(editionPath.begin(), editionPath.end(), '/', '\\');
        return editionPath;
    }

    return "";
}

std::string ProcessDispatcher::ValidateVSNinjaPath(const std::string& path)
{
    std::filesystem::path ninjaPath(path);

    if (std::filesystem::exists(ninjaPath))
        ninjaPath /= "Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja";
    if (std::filesystem::exists(ninjaPath))
    {
        std::string ninjaPathStr = ninjaPath.string();
        std::replace(ninjaPathStr.begin(), ninjaPathStr.end(), '/', '\\');
        return ninjaPathStr;
    }
    return "";
}

std::string ProcessDispatcher::ValidateVCEnvPath(const std::string& path)
{
    std::filesystem::path vcEnvPath(path);

    if (std::filesystem::exists(vcEnvPath))
        vcEnvPath /= "VC/Auxiliary/Build/vcvars64.bat";
    if (std::filesystem::exists(vcEnvPath))
    {
        std::string ninjaPathStr = vcEnvPath.string();
        std::replace(ninjaPathStr.begin(), ninjaPathStr.end(), '/', '\\');
        return ninjaPathStr;
    }
    return "";
}

void ProcessDispatcher::InitVCEnv(const std::string& cmd)
{
    HANDLE r, w;
    //cmd.exe / c \"vcvars64.bat >nul && set\"
    std::string treatedCmd = cmd;
    treatedCmd = std::regex_replace(treatedCmd, std::regex(" "), "^ ");
    std::string command = "cmd.exe /c \"" + treatedCmd + " > nul && set\"";
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&r, &w, &sa, 0);
    SetHandleInformation(r, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION pi{};
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.hStdOutput = w;
    si.hStdError = w;
    si.dwFlags |= STARTF_USESTDHANDLES;

    CreateProcessA(
        NULL,
        (LPSTR)command.c_str(),
        NULL, NULL, TRUE,
        CREATE_NO_WINDOW,
        NULL, NULL,
        &si, &pi
    );

    CloseHandle(w);

    std::string output;
    char buffer[4096];
    DWORD read;
    while (ReadFile(r, buffer, sizeof(buffer), &read, NULL) && read > 0) {
        output.append(buffer, read);
    }

    CloseHandle(r);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    ApplyEnvironment(output);

    //char* path = new char[32768];
    //GetEnvironmentVariableA("PATH", path, 32768);
    //std::cout << path << std::endl;
    //delete[] path;

}

void ProcessDispatcher::ApplyEnvironment(const std::string& envText)
{
    size_t pos = 0;
    size_t len = envText.size();

    while (pos < len)
    {
        // Find end of line
        size_t end = envText.find('\n', pos);
        if (end == std::string::npos)
            end = len;

        // Extract line (trim CR if present)
        std::string line = envText.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        pos = end + 1;

        // Find VAR=VALUE
        size_t eq = line.find('=');
        if (eq == std::string::npos)
            continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (!key.empty())
        {
            // Apply to this process' environment
            SetEnvironmentVariableA(key.c_str(), value.c_str());
        }
    }
}

bool ProcessDispatcherSource::SearchExecutableLocationOnWindows(std::string_view programName)
{
    std::string programFullName = programName.data();
    programFullName = programFullName + ".exe";
	char buffer[MAX_PATH];
	bool found = false;
	if (SearchPathA(nullptr, programFullName.c_str(), nullptr, MAX_PATH, buffer, nullptr)) {
		found = true;
	}

	return found;
}

bool ProcessDispatcherSource::ExecuteCommandOnWindows(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory)
{
    std::string path = workingDirectory;
    std::replace(path.begin(), path.end(), '/', '\\');
	SetCurrentDirectoryA(path.c_str());
    std::string programName = command.data();
    //programName = programName + ".exe";
    std::string cmd = programName;
    for (auto& a : arguments)
        cmd += " " + a;

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    //detached process
    //uint32_t processCreationFlags = DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP;
    //attached process
    uint32_t processCreationFlags = 0;

    if (!CreateProcessA(
        nullptr,                             // If program is on PATH, pass NULL and include exe name in buffer
        (LPSTR)cmd.c_str(),                  // Mutable command line
        nullptr, nullptr,                    // Security
        FALSE,                               // Inherit handles
        processCreationFlags,                // Flags
        nullptr,                             // Environment
        path.c_str(),                        // Working dir
        &si,
        &pi))
    {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    std::string executableLocation = ProcessDispatcher::GetExecutableLocation();
    SetCurrentDirectoryA(executableLocation.c_str());

    return exitCode == 0;
}

void ProcessDispatcherSource::AppendDirectoryToPathOnWindows(std::string_view directory)
{
    char* path = new char[32768];
    GetEnvironmentVariableA("PATH", path, 32768);
	std::stringstream pathStream;
	std::string pathPart;
    pathStream << directory << ";" << path;
    std::string treatedPath = pathStream.str();
    delete[] path;
	std::replace(treatedPath.begin(), treatedPath.end(), '/', '\\');
	SetEnvironmentVariableA("PATH", treatedPath.c_str());
}

#elif defined(__linux__) || defined(__FreeBSD__)
bool ProcessDispatcherSource::SearchExecutableLocationOnUnix(std::string_view programName)
{
    // If it contains a slash, it must be an actual file path
    if (strchr(programName.data(), '/')) {
        return access(programName.data(), X_OK) == 0;
    }

    // Otherwise search PATH manually
    const char* path = getenv("PATH");
    if (!path) return false;

    std::string p(path);
    std::stringstream ss(p);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        std::string full = dir + "/" + programName.data();
        if (access(full.c_str(), X_OK) == 0)
            return true;
    }

    return false;
}

bool ProcessDispatcherSource::ExecuteCommandOnUnix(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory)
{
    std::vector<const char*> c_args;
    //argv.push_back(const_cast<char*>(command.data()));
    c_args.push_back(command.data());
    for (auto& a : arguments)
        c_args.push_back(a.c_str());
    c_args.push_back(nullptr);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return false;
    }

    if (pid == 0) {
        // --- Child: runs the external program ---
        std::filesystem::current_path(workingDirectory.data());
        execvp(command.data(), (char *const *)c_args.data());

        // Only executed if execvp fails
        perror("execvp failed");
        _exit(1);
    }

    // --- Parent: wait for the child to finish ---
    int status = 0;
    waitpid(pid, &status, 0);

    std::cout << "Program executed" << "\n";
    return true;
}

void ProcessDispatcherSource::AppendDirectoryToPathOnUnix(std::string_view directory)
{
	std::string path = getenv("PATH");
	std::stringstream pathStream(path);
	std::string pathPart;
	pathStream << ":" << directory.data();
	std::string treatedPath = pathStream.str();
	setenv("PATH", treatedPath.c_str(), 1);
}

#endif
    