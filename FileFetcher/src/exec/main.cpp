#include <iostream>
#include "Downloader.hpp"
#include "CurlStarter.hpp"
#include "CertDownloader.hpp"
#include "FileHandler.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

static bool is_path_safe(const std::filesystem::path& p)
{
    if (p.is_absolute())
        return false;

    for (const auto& part : p) {
        if (part == "..")
            return false;
    }
    return true;
}

int extract_zip_from_memory(const void* data,
    std::size_t size,
    const fs::path& out_dir)
{
    struct archive* a = archive_read_new();
    if (!a)
        return -1;

    archive_read_support_format_zip(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_memory(a, data, size) != ARCHIVE_OK) {
        std::cerr << archive_error_string(a) << '\n';
        archive_read_free(a);
        return -1;
    }

    fs::create_directories(out_dir);

    archive_entry* entry;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        fs::path rel_path = archive_entry_pathname(entry);

        if (!is_path_safe(rel_path)) {
            std::cerr << "Skipping unsafe path: " << rel_path << '\n';
            archive_read_data_skip(a);
            continue;
        }

        fs::path full_path = out_dir / rel_path;

        if (archive_entry_filetype(entry) == AE_IFDIR) {
            fs::create_directories(full_path);
            continue;
        }

        fs::create_directories(full_path.parent_path());

        std::ofstream out(full_path, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open " << full_path << '\n';
            archive_read_free(a);
            return -1;
        }

        std::vector<char> buffer(8192);
        la_ssize_t n;

        while ((n = archive_read_data(a, buffer.data(), buffer.size())) > 0) {
            out.write(buffer.data(), n);
        }

        if (n < 0) {
            std::cerr << archive_error_string(a) << '\n';
            archive_read_free(a);
            return -1;
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

    archive_read_free(a);
    return 0;
}

#ifndef _WIN32

std::string expand_home()
{
    std::string home = std::getenv("HOME");
    if (home.empty())
        throw std::runtime_error("HOME is not set");
    return home;
}

#endif

int main(int argc, char* argv[]) {

    CURLStarter::InitCurl();
#if defined(_WIN32)
    CURLStarter::SetCertificateLocation("C:\\curl_config\\certs\\curl-ca-bundle.crt");
    CertDownloader::AssertCertHash("C:\\curl_config\\certs\\");
#else
    CURLStarter::SetCertificateLocation(expand_home() + "/curl_config/certs/curl-ca-bundle.crt");
    CertDownloader::AssertCertHash(expand_home() + "/curl_config/certs/");
#endif
    Downloader downloader;
    std::shared_ptr<RawBuffer> m_OutputBuffer;
    m_OutputBuffer.reset(new RawBuffer());
    try {
        downloader.DownloadFile("https://globalcdn.nuget.org/packages/microsoft.direct3d.d3d12.1.618.5.nupkg");
    }
	catch (InvalidCertLocation& e) {
		std::cout << e.what() << std::endl;
        CertDownloader::DownloadCert();
#       if defined(_WIN32)
        CertDownloader::AssertCertHash("C:\\curl_config\\certs\\");
#       else
        CertDownloader::AssertCertHash(expand_home() + "/curl_config/certs/");
#       endif
		return 65;
	}
    catch (InvalidCert& e)
    {
		std::cout << e.what() << std::endl;
        FileHandler::DeleteFileAt("C:\\curl_config\\certs\\curl-ca-bundle.crt");
		FileHandler::DeleteFileAt("C:\\curl_config\\certs\\curl-ca-bundle.crt.sha256");
        std::cout << "You can rerun the program to try again or manually download the certificates at https://curl.se/ca/cacert.pem and https://curl.se/ca/cacert.pem.sha256" << std::endl;
		return 65;
    }
    downloader.PackMemoryChunks(&m_OutputBuffer);
    /*std::string filename = "d3d12_sdk.zip";
    FileHandler::WriteBinFile(filename, m_OutputBuffer);*/
	extract_zip_from_memory(m_OutputBuffer->GetData(), m_OutputBuffer->GetSize(), "d3d12_sdk");
    CURLStarter::ShutdownCurl();
    return 0;
}