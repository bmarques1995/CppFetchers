#include "Utils.hpp"
#include <filesystem>
#include <regex>
#include <fmt/core.h>
#include "Placeholders.hpp"
#include "FileHandler.hpp"

#ifdef WIN32
const std::string Utils::s_SystemName = "windows";
#elif defined(__linux__)
const std::string Utils::s_SystemName = "linux";
#elif defined(__FreeBSD__)
const std::string Utils::s_SystemName = "freebsd";
#endif

std::string Utils::GetAbsoluteLocation(const std::string& path)
{
	std::filesystem::path absolutePath = std::filesystem::absolute(path);
	return absolutePath.string();
}

std::string Utils::ProcessFlag(const std::string& flag)
{
	if (flag.starts_with("\""))
	{
		return ProcessQuotedFlag(flag);
	}
	else
	{
		return ProcessRawFlag(flag);
	}
}

#ifdef WIN32

std::string Utils::WindowsPathToMsys(const std::string& win)
{
	if (win.size() < 3) return win;

	// Check drive-letter format: X:\...
	if (win[1] == ':' && (win[2] == '\\' || win[2] == '/')) {
		char drive = std::tolower(win[0]);
		std::string rest = win.substr(2);

		// Replace '\' with '/'
		for (auto& c : rest)
			if (c == '\\') c = '/';

		return "/" + std::string(1, drive) + rest;
	}

	// Not a drive path → return as-is
	return win;
}

std::string Utils::EscapeChars(const std::string& input)
{
	// 3. Escape spaces
	std::string escaped;
	escaped.reserve(input.size());
	for (char c : input) {
		if (c == ' ' || c == '(' || c == ')') {
			escaped += "\\";
		}
		escaped += c;
	}

	return escaped;
}

#endif // 

std::string Utils::SaveShellCommand(const std::string& command, const std::string& type)
{
	std::string shell = Placeholders::GetPlaceholder("module_path");
	shell += "/gen_command.";
	shell += type;
	shell += ".sh";
	std::string script = command;
	script = script.substr(1, script.length() - 2);
	FileHandler::WriteTextFile(shell, script);
	return shell;
}

std::string Utils::ProcessRawFlag(const std::string& flag)
{
	std::string input = flag;

	std::string treatedFlag = flag;

	{
		std::regex blockRegex(R"(\[([A-Za-z0-9_]+):(.*?)\])");
		std::smatch match;
		std::string processed;
		std::string temp = input;

		auto begin = temp.cbegin();
		auto end = temp.cend();

		while (std::regex_search(begin, end, match, blockRegex)) {
			processed.append(match.prefix().str());

			std::string mode = match[1].str();
			std::string inside = match[2].str();

			if (mode.compare(Placeholders::GetPlaceholder("build_mode")) == 0)
				processed.append(inside);
			else
				return "";

			begin = match.suffix().first;
			input = processed;
		}
	}

	{
		std::regex re(R"(\{([^}]*)\})");
		std::smatch match;
		std::string placeholder;
		if (std::regex_search(input, match, re)) {
			placeholder = match[1];   // "install_prefix"
			treatedFlag = std::regex_replace(input, re, "{}");
			std::string finalFlag = fmt::format(fmt::runtime(treatedFlag), Placeholders::GetPlaceholder(placeholder));
			return finalFlag;
		}
	}

	return input;
}

std::string Utils::ProcessQuotedFlag(const std::string& flag)
{
	std::string unquotedFlag = flag.substr(1, flag.size() - 2);
	std::vector<std::string> splitFlag = Utils::SplitRespectingEscapedSpaces(unquotedFlag);

	std::string processedFlag = "\"";
	for (std::string part : splitFlag)
	{
		std::string flag = ProcessRawFlag(part);
		processedFlag += ProcessRawFlag(part);
		if(flag.compare("") != 0)
			processedFlag += " ";
	}

	processedFlag = processedFlag.substr(0, processedFlag.size() - 1);
	processedFlag += "\"";
	return processedFlag;
}

std::vector<std::string> Utils::SplitRespectingEscapedSpaces(const std::string& input)
{
	std::vector<std::string> result;
	std::string current;
	bool escape = false;

	for (size_t i = 0; i < input.size(); ++i) {
		char c = input[i];

		if (escape) {
			// Handle escaped characters
			if (c == ' ') {
				current.push_back(' '); // "\ " becomes " "
			}
			else {
				current.push_back('\\'); // keep the backslash if not escaping a space
				current.push_back(c);
			}
			escape = false;
		}
		else if (c == '\\') {
			escape = true; // next char is escaped
		}
		else if (c == ' ') {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		}
		else {
			current.push_back(c);
		}
	}

	if (!current.empty())
		result.push_back(current);

	return result;
}
