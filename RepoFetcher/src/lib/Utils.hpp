#pragma once

#include <string>
#include <vector>
#include <stdexcept>

class PlaceholderException : public std::exception
{
public:
	PlaceholderException(const std::string& message);
	const char* what() const noexcept override;
private:
	std::string m_Message;
};

class BuildModeConditionException : public std::exception
{
public:
	BuildModeConditionException(const std::string& message);
	const char* what() const noexcept override;
private:
	std::string m_Message;
};

class Utils
{
public:
	static std::string ExpandPath(const std::string& path);
	static std::string GetAbsoluteLocation(const std::string& path);
	static std::string ProcessFlag(const std::string& flag);

#ifdef WIN32
	static std::string WindowsPathToMsys(const std::string& win);
	static std::string EscapeCharsForPath(const std::string& input);
#endif

	static std::string ProcessPlaceholders(const std::string& input);
	static std::string ProcessBuildModeConditions(const std::string& input, const std::string& buildMode);
	static std::string NormalizeString(const std::string& input);
	static std::string NormalizeFlag(const std::string& flag);

	static std::string SaveShellCommand(const std::string& command, const std::string& type);

	static const std::string s_SystemName;
private:
	static std::string ProcessRawFlag(const std::string& flag);
	static std::string ProcessQuotedFlag(const std::string& flag);
	static std::vector<std::string> SplitRespectingEscapedSpaces(const std::string& input);
};