#pragma once

#include <string>
#include <vector>

class ProcessDispatcher
{
public:
	static bool SearchExecutableLocation(std::string_view programName);
	static bool ExecuteCommand(std::string_view command, const std::vector<std::string>& arguments, const std::string& workingDirectory = "./");
}; 