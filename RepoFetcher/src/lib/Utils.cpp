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

std::string Utils::EscapeCharsForPath(const std::string& input)
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

std::string Utils::ProcessPlaceholders(const std::string& input)
{
	std::string out;
	out.reserve(input.size());

	for (size_t i = 0; i < input.size(); ) {
		char c = input[i];

		// Escape handling
		if (c == '\\') {
			if (i + 1 >= input.size())
				throw PlaceholderException("Dangling escape");

			char next = input[i + 1];

			if (next == '{' || next == '}') {
				out.push_back(next);
				i += 2;
				continue;
			}
			out.push_back(next);
			i += 2;
			continue;
		}

		if (c == '{') {
			size_t start = ++i;

			if (i >= input.size())
				throw PlaceholderException("Unclosed '{'");

			while (i < input.size() && input[i] != '}')
				++i;

			if (i == input.size())
				throw PlaceholderException("Unclosed '{'");

			if (i == start)
				throw PlaceholderException("Empty placeholder {}");

			std::string key = input.substr(start, i - start);
			++i; // consume '}'

			auto value = Placeholders::GetPlaceholder(key);

			out += "{}";
			out = fmt::format(fmt::runtime(out), value);
			continue;
		}

		if (c == '}') {
			throw PlaceholderException("Unexpected '}'");
		}

		out.push_back(c);
		++i;
	}

	return out;
}

std::string Utils::ProcessBuildModeConditions(const std::string& input, const std::string& buildMode)
{
	std::string out;
	out.reserve(input.size());

	for (size_t i = 0; i < input.size(); ) {
		char c = input[i];

		// Escape handling: ONLY \[
		if (c == '\\') {
			if (i + 1 >= input.size())
				throw BuildModeConditionException("Dangling escape");

			char next = input[i + 1];

			if (next == '[') {
				out.push_back('[');
				i += 2;
				continue;
			}

			// All other escapes: literal next char
			out.push_back(next);
			i += 2;
			continue;
		}

		// Conditional start
		if (c == '[') {
			size_t start = ++i;

			if (i >= input.size())
				throw BuildModeConditionException("Unclosed '['");

			while (i < input.size() && input[i] != ']')
				++i;

			if (i == input.size())
				throw BuildModeConditionException("Unclosed '['");

			std::string block = input.substr(start, i - start);
			++i;

			auto colon = block.find(':');
			if (colon == std::string::npos)
				throw BuildModeConditionException("Conditional missing ':'");

			std::string mode = block.substr(0, colon);
			std::string value = block.substr(colon + 1);
			if (mode != "Debug" && mode != "Release")
				throw BuildModeConditionException("Invalid build mode: " + mode);

			if (mode == buildMode)
				out += value;

			continue;
		}

		// Stray closing bracket
		if (c == ']') {
			throw BuildModeConditionException("Unexpected ']'");
		}

		out.push_back(c);
		++i;
	}

	return out;
}

std::string Utils::NormalizeString(const std::string& input)
{
	std::string out;
	out.reserve(input.size());

	bool inWhitespace = false;

	for (char c : input) {
		bool isWs =
			c == ' ' ||
			c == '\t' ||
			c == '\n' ||
			c == '\r';

		if (isWs) {
			// Emit a single space only if we've already written something
			// and we're not already in a whitespace run
			if (!inWhitespace && !out.empty()) {
				out.push_back(' ');
				inWhitespace = true;
			}
		}
		else {
			out.push_back(c);
			inWhitespace = false;
		}
	}

	// Remove trailing space if present
	if (!out.empty() && out.back() == ' ')
		out.pop_back();

	return out;
}

std::string Utils::NormalizeFlag(const std::string& flag)
{
	return NormalizeString(flag);
}

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
	input = ProcessBuildModeConditions(input, Placeholders::GetPlaceholder("build_mode"));
	input = ProcessPlaceholders(input);
	return input;
}

std::string Utils::ProcessQuotedFlag(const std::string& flag)
{
	std::string unquotedFlag = flag.substr(1, flag.size() - 2);
	std::vector<std::string> splitFlag = Utils::SplitRespectingEscapedSpaces(unquotedFlag);

	std::string processedFlag = "\"";
	for (std::string part : splitFlag)
	{
		std::string processedFlagPart = ProcessRawFlag(part);
		std::string flag = processedFlagPart;
		processedFlag += processedFlagPart;
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

PlaceholderException::PlaceholderException(const std::string& message) :
	m_Message(message)
{
}

const char* PlaceholderException::what() const noexcept
{
	return m_Message.c_str();
}

BuildModeConditionException::BuildModeConditionException(const std::string& message) :
	m_Message(message)
{
}

const char* BuildModeConditionException::what() const noexcept
{
	return m_Message.c_str();
}
