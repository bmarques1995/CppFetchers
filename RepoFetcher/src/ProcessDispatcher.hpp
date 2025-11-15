#pragma once

#include <string>
#include <vector>

class ProcessDispatcher
{
public:
	static bool SearchExecutableLocation(std::string_view programName);
	static bool ExecuteCommand(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory = "./");

	static void SetExecutableLocation(std::string_view location);
	static const std::string& GetExecutableLocation();
private:
	static std::string s_ExecutableLocation;
}; 