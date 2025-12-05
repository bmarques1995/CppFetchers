#pragma once

#include <string>
#include <vector>

class Utils
{
public:
	static std::string GetAbsoluteLocation(const std::string& path);
	static std::string ProcessFlag(const std::string& flag);

#ifdef WIN32
	static std::string WindowsPathToMsys(const std::string& win);
	static std::string EscapeChars(const std::string& input);
#endif

	static std::string SaveShellCommand(const std::string& command, const std::string& type);

	static const std::string s_SystemName;
private:
	static std::string ProcessRawFlag(const std::string& flag);
	static std::string ProcessQuotedFlag(const std::string& flag);
	static std::vector<std::string> SplitRespectingEscapedSpaces(const std::string& input);
};