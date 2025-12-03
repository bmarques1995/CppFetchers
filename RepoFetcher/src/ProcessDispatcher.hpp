#pragma once

#include <string>
#include <vector>

//C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.50.35717/bin/Hostx64/x64/cl.exe
//C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja

class ProcessDispatcher
{
public:
	static bool SearchExecutableLocation(std::string_view programName);
	static bool ExecuteCommand(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory = "./");

	static void AppendDirectoryToPath(std::string_view directory);

	static void SetExecutableLocation(std::string_view location);
	static const std::string& GetExecutableLocation();

#ifdef WIN32

	static std::string ExtractVSBasePath(const std::string& path);
	static std::string ValidateVSNinjaPath(const std::string& path);
	static std::string ValidateVCEnvPath(const std::string& path);
	static void InitVCEnv(const std::string& cmd);

	static void ApplyVSEnvironment();
#endif

private:

#ifdef WIN32

	static void ApplyEnvironment(const std::string& envText);

#endif
	static std::string s_ExecutableLocation;
}; 