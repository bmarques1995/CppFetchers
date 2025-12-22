#include "ArchiveOut.hpp"
#include <iostream> 

namespace fs = std::filesystem;

ArchiveOut::ArchiveOut(const std::shared_ptr<RawBuffer>& buffer, std::string_view outputDir) :
	m_OutputDir(outputDir), m_Archive(nullptr)
{
    m_Archive = archive_read_new();
    if (!m_Archive)
        return;

    archive_read_support_format_tar(m_Archive);
    archive_read_support_format_zip(m_Archive);
    archive_read_support_filter_all(m_Archive);

    if (archive_read_open_memory(m_Archive, buffer->GetData(), buffer->GetSize()) != ARCHIVE_OK) {
        std::cerr << archive_error_string(m_Archive) << '\n';
        archive_read_free(m_Archive);
        return;
    }
}

ArchiveOut::~ArchiveOut()
{
    archive_read_free(m_Archive);
}

void ArchiveOut::SaveFiles()
{
    fs::create_directories(m_OutputDir);

    archive_entry* entry;
	

    while (archive_read_next_header(m_Archive, &entry) == ARCHIVE_OK) {

        fs::path rel_path = RebaseRoot(archive_entry_pathname(entry), m_OutputDir);

        if (!IsPathSafe(rel_path)) {
            std::cerr << "Skipping unsafe path: " << rel_path << '\n';
            archive_read_data_skip(m_Archive);
            continue;
        }

        fs::path full_path = rel_path;

        if (archive_entry_filetype(entry) == AE_IFDIR) {
            fs::create_directories(full_path);
            continue;
        }

        auto entryType = archive_entry_filetype(entry);
        fs::create_directories(full_path.parent_path());

        std::ofstream out(full_path, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open " << full_path << '\n';
            archive_read_free(m_Archive);
            return;
        }

        std::vector<char> buffer(8192);
        la_ssize_t n;

        while ((n = archive_read_data(m_Archive, buffer.data(), buffer.size())) > 0) {
            out.write(buffer.data(), n);
        }

        if (n < 0) {
            std::cerr << archive_error_string(m_Archive) << '\n';
            archive_read_free(m_Archive);
        }

        out.close();

#ifndef _WIN32
        /* Restore permissions (POSIX only) */
        std::error_code ec;
        fs::permissions(
            full_path,
            static_cast<fs::perms>(archive_entry_perm(entry)),
            fs::perm_options::replace,
            ec
        );
#endif
    }
}

bool ArchiveOut::IsPathSafe(const std::filesystem::path& p)
{
    if (p.is_absolute())
        return false;

    for (const auto& part : p) {
        if (part == "..")
            return false;
    }
    return true;
}

std::filesystem::path ArchiveOut::RebaseRoot(const fs::path& p, const fs::path& new_root)
{
    fs::path result = new_root;
    auto it = p.begin();

    if (it != p.end()) {
        ++it; // skip original root component
    }

    for (; it != p.end(); ++it) {
        result /= *it;
    }

    return result;
}

