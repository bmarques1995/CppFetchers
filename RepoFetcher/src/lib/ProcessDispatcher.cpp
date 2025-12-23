#include "ProcessDispatcher.hpp"

#include <cstring>
#include <sstream>
#include <filesystem>

#ifdef WIN32
#include "Placeholders.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <windows.h>
#include <regex>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

std::string ProcessDispatcher::s_ExecutableLocation = "";

namespace ProcessDispatcherSource
{
#ifdef WIN32
	std::string SearchExecutableLocationOnWindows(std::string_view programName);
	bool ExecuteCommandOnWindows(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory);
    void AppendDirectoryToPathOnWindows(std::string_view directory);
    void AppendVariableOnWindows(std::string_view key, std::string_view value);
#else
    std::string SearchExecutableLocationOnUnix(std::string_view programName);
    bool ExecuteCommandOnUnix(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory);
    void AppendDirectoryToPathOnUnix(std::string_view directory);
    static void SanitizeUnixPath(const std::string& inputPath, std::string* outputPath);
    void AppendVariableOnUnix(std::string_view key, std::string_view value);
#endif // WIN32
}

std::string ProcessDispatcher::SearchExecutableLocation(std::string_view programName)
{
#ifdef WIN32
	return ProcessDispatcherSource::SearchExecutableLocationOnWindows(programName);
#else
    return ProcessDispatcherSource::SearchExecutableLocationOnUnix(programName);
#endif
}

bool ProcessDispatcher::ExecuteCommand(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory)
{
#ifdef WIN32
	return ProcessDispatcherSource::ExecuteCommandOnWindows(command, arguments, workingDirectory);
#else
    return ProcessDispatcherSource::ExecuteCommandOnUnix(command, arguments, workingDirectory);
#endif
}

void ProcessDispatcher::AppendDirectoryToPath(std::string_view directory)
{
#ifdef WIN32
    return ProcessDispatcherSource::AppendDirectoryToPathOnWindows(directory);
#else
    return ProcessDispatcherSource::AppendDirectoryToPathOnUnix(directory);
#endif
}

void ProcessDispatcher::AppendVariable(std::string_view key, std::string_view value)
{
#ifdef WIN32
    return ProcessDispatcherSource::AppendVariableOnWindows(key, value);
#else
    return ProcessDispatcherSource::AppendVariableOnUnix(key, value);
#endif
}

void ProcessDispatcher::FilterPath()
{
#ifdef WIN32
    FilterPathOnWindows();
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

void ProcessDispatcher::ApplyPathPlaceholder()
{
    char* path = new char[65536];
    GetEnvironmentVariableA("PATH", path, 65536);
    Placeholders::SetPlaceholder("path", path);
    std::string msysPath = ConvertFullPathListToMsys(path);
    std::string escapedPath = Utils::EscapeCharsForPath(msysPath);
    Placeholders::SetPlaceholder("msys_escaped_path", escapedPath);
    delete[] path;
}

void ProcessDispatcher::FilterPathOnWindows()
{
    char* path = new char[32768];
    GetEnvironmentVariableA("PATH", path, 32768);
    std::stringstream pathOutStream;
    std::stringstream pathInStream(path);
    delete[] path;
    std::string element;
    while (std::getline(pathInStream, element, ';')) {
        std::string lower = element;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (!lower.ends_with("\\microsoft\\windowsapps") &&
            !lower.ends_with("\\microsoft\\windowsapps\\"))
        {
            pathOutStream << element << ";";
        }
    }
    std::string treatedPath = pathOutStream.str();
    std::replace(treatedPath.begin(), treatedPath.end(), '/', '\\');
    SetEnvironmentVariableA("PATH", treatedPath.c_str());
}

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

std::string ProcessDispatcher::ExtractMSVCLibsPath(const std::string& path)
{
    std::filesystem::path msvcLibsPath(path);
    msvcLibsPath = msvcLibsPath.parent_path().parent_path().parent_path().parent_path();
    msvcLibsPath /= "lib\\x64";
    std::string msvcLibsPathStr = msvcLibsPath.string();
    return msvcLibsPathStr;
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
    
    //cmd.exe / c \"vcvars64.bat >nul && set\"
    std::string treatedCmd = cmd;
    treatedCmd = std::regex_replace(treatedCmd, std::regex(" "), "^ ");
    std::string command = "cmd.exe /c \"" + treatedCmd + " > nul && set\"";
    std::string output;

	ExecuteOutputCommand(command, &output);
    ApplyEnvironment(output);
    ApplyPathPlaceholder();
}

void ProcessDispatcher::ApplyVSEnvironment()
{
    std::string compilerPath = Placeholders::GetPlaceholder("compiler_path");
    if (!compilerPath.empty())
    {
        std::string vsBasePath = ProcessDispatcher::ExtractVSBasePath(compilerPath);
        std::string vcEnvPath = ProcessDispatcher::ValidateVCEnvPath(vsBasePath);
        std::string ninjaPath = ProcessDispatcher::ValidateVSNinjaPath(vsBasePath);
        std::string msvcLibsPath = ProcessDispatcher::ExtractMSVCLibsPath(compilerPath);
        ProcessDispatcher::InitVCEnv(vcEnvPath);
        ProcessDispatcher::AppendDirectoryToPath(ninjaPath);
		ProcessDispatcher::AppendDirectoryToPath(msvcLibsPath);
    }
    else
    {
        ApplyPathPlaceholder();
    }
}

void ProcessDispatcher::ExecuteOutputCommand(std::string_view command, std::string* output)
{
    HANDLE r, w;
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
        (LPSTR)command.data(),
        NULL, NULL, TRUE,
        CREATE_NO_WINDOW,
        NULL, NULL,
        &si, &pi
    );

    CloseHandle(w);
    char* path = new char[65536];
    DWORD read;
    while (ReadFile(r, path, 65536, &read, NULL) && read > 0) {
        (*output).append(path, read);
    }

    CloseHandle(r);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] path;
}


void ProcessDispatcher::ApplyEnvironment(const std::string& envText)
{
    std::unordered_map<std::string, std::string> varMap;
    ExtractKeys(&varMap, envText);
	for (const auto& var : varMap)
	{
		SetEnvironmentVariableA(var.first.c_str(), var.second.c_str());
	}
}

void ProcessDispatcher::ExtractKeys(std::unordered_map<std::string, std::string>* map, const std::string& text)
{
    size_t pos = 0;
    size_t len = text.size();

    while (pos < len)
    {
        // Find end of line
        size_t end = text.find('\n', pos);
        if (end == std::string::npos)
            end = len;

        // Extract line (trim CR if present)
        std::string line = text.substr(pos, end - pos);
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
			(*map)[key] = value;
            // Apply to this process' environment
            //SetEnvironmentVariableA(key.c_str(), value.c_str());
        }
    }
}



std::string ProcessDispatcher::ConvertFullPathListToMsys(const std::string& input)
{
    std::stringstream ss(input);
    std::string item;
    std::vector<std::string> converted;

    while (std::getline(ss, item, ';')) {
        converted.push_back(Utils::WindowsPathToMsys(item));
    }

    // Re-join using ':' (MSYS2 works with both; ':' is native)
    std::string result;
    for (size_t i = 0; i < converted.size(); ++i) {
        if (i > 0) result += ":";
        result += converted[i];
    }
    return result;
}

std::string ProcessDispatcherSource::SearchExecutableLocationOnWindows(std::string_view programName)
{
    std::string programFullPath = "";
    std::string programFullName = programName.data();
    programFullName = programFullName + ".exe";
	char buffer[MAX_PATH];
	bool found = false;
	if (SearchPathA(nullptr, programFullName.c_str(), nullptr, MAX_PATH, buffer, nullptr)) {
		programFullPath = buffer;
	}

	return programFullPath;
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
    AppendVariableOnWindows("PATH", directory);
}

void ProcessDispatcherSource::AppendVariableOnWindows(std::string_view key, std::string_view value)
{
    char* currentValue = new char[32768];
    GetEnvironmentVariableA(key.data(), currentValue, 32768);
    std::stringstream valueStream;
    valueStream << value << ";" << currentValue;
    std::string treatedValue = valueStream.str();
    delete[] currentValue;
    std::replace(treatedValue.begin(), treatedValue.end(), '/', '\\');
    SetEnvironmentVariableA(key.data(), treatedValue.c_str());
}

#else
std::string ProcessDispatcherSource::SearchExecutableLocationOnUnix(std::string_view programName)
{
    std::string programFullPath = "";
    // If it contains a slash, it must be an actual file path
    if (strchr(programName.data(), '/')) {
        if (access(programName.data(), X_OK) == 0)
        {
            programFullPath = programName;
            return programFullPath;
        }
    }

    // Otherwise search PATH manually
    const char* path = getenv("PATH");
    if (!path) return "";
    std::string sanitizedPath;
    SanitizeUnixPath(path, &sanitizedPath);

    std::stringstream ss(sanitizedPath);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        std::string full = dir + "/" + programName.data();
        if (access(full.c_str(), X_OK) == 0)
        {
            programFullPath = full;
        }
    }

    return programFullPath;
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

    return true;
}

void ProcessDispatcherSource::AppendDirectoryToPathOnUnix(std::string_view directory)
{
    AppendVariableOnUnix("PATH", directory);
}

void ProcessDispatcherSource::SanitizeUnixPath(const std::string& inputPath, std::string* outputPath)
{
    std::string p(inputPath);
    std::stringstream ss(p);
    std::stringstream output;
    std::string dir;
    while (std::getline(ss, dir, ':')) {
        if(dir.rfind("/mnt/", 0) != 0)
        {
            output << dir << ":";
        }
    }
    *outputPath = output.str();
    outputPath->pop_back();
}

void ProcessDispatcherSource::AppendVariableOnUnix(std::string_view key, std::string_view value)
{
    const char* rawVariable = getenv(key.data());
    std::string variable = rawVariable == nullptr ? "" : rawVariable;
    std::stringstream varStream;
    std::string pathPart;
    varStream << value << ":" << variable;
    std::string treatedValue = varStream.str();
    setenv(key.data(), treatedValue.c_str(), 1);
}

#endif
    