#include "Utils.hpp"
#include <filesystem>
#include <iostream>

std::string Utils::GetAbsoluteLocation(const std::string& path)
{
	if (path.starts_with("."))
	{
		std::filesystem::path absolutePath = std::filesystem::absolute(path);
		std::cout << absolutePath.string() << "\n";
		return absolutePath.string();
	}
	else
		return path;
}
