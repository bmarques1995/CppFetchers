#pragma once

#include <string>
#include <vector>
#ifdef WIN32
#include <unordered_map>
#else
#endif

//C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.50.35717/bin/Hostx64/x64/cl.exe
//C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja

class ProcessDispatcher
{
public:
	static std::string SearchExecutableLocation(std::string_view programName);
	static bool ExecuteCommand(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory = "./");

	static void AppendDirectoryToPath(std::string_view directory);
	static void AppendVariable(std::string_view key, std::string_view value);

	static void FilterPath();

	static void SetExecutableLocation(std::string_view location);
	static const std::string& GetExecutableLocation();

#ifdef WIN32

	static void ApplyPathPlaceholder();
	static void FilterPathOnWindows();
	static std::string ExtractVSBasePath(const std::string& path);
	static std::string ValidateVSNinjaPath(const std::string& path);
	static std::string ValidateVCEnvPath(const std::string& path);
	static void InitVCEnv(const std::string& cmd);
	static void ExtractKeys(std::unordered_map<std::string, std::string>* map, const std::string& text);

	static void ApplyVSEnvironment();
	static void ExecuteOutputCommand(std::string_view command, std::string* output);
#endif

private:

#ifdef WIN32
	static void ApplyEnvironment(const std::string& envText);
    static std::string ConvertFullPathListToMsys(const std::string& input);
#else
	
#endif
	static std::string s_ExecutableLocation;
}; 