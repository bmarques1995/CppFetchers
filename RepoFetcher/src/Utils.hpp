#pragma once

#include <string>

class Utils
{
public:
	static std::string GetAbsoluteLocation(const std::string& path);
	static std::string ProcessFlag(const std::string& flag);

	static const std::string s_SystemName;
};