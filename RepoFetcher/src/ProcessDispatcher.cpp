#include "ProcessDispatcher.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <filesystem>

#ifdef WIN32
#include <windows.h>
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
#elif defined(__linux__) || defined(__FreeBSD__)
    bool SearchExecutableLocationOnUnix(std::string_view programName);
    bool ExecuteCommandOnUnix(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory);
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

void ProcessDispatcher::SetExecutableLocation(std::string_view location)
{
    s_ExecutableLocation = location;
}

const std::string& ProcessDispatcher::GetExecutableLocation()
{
    return s_ExecutableLocation;
}

#ifdef WIN32
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

#endif
    