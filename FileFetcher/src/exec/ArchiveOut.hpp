#pragma once

#include <string>
#include "RawBuffer.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <fstream>
#include <vector>

class ArchiveOut
{
public:
	ArchiveOut(const std::shared_ptr<RawBuffer>& buffer, std::string_view outputDir);
	~ArchiveOut();

	void SaveFiles();

private:
	bool IsPathSafe(const std::filesystem::path& p);
	std::filesystem::path RebaseRoot(const std::filesystem::path& p, const std::filesystem::path& new_root);
	std::filesystem::path m_ArchivePath;
	archive* m_Archive;
	std::string m_OutputDir;
};