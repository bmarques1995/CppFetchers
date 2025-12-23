#include <iostream>
#include "Downloader.hpp"
#include "CurlStarter.hpp"
#include "CertDownloader.hpp"
#include "FileHandler.hpp"
#include "ArchiveOut.hpp"
#include "Placeholders.hpp"
#include "Utils.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>

#ifndef _WIN32

std::string expand_home()
{
    std::string home = std::getenv("HOME");
    if (home.empty())
        throw std::runtime_error("HOME is not set");
    return home;
}

#endif

int main(int argc, char* argv[])
{
    
    if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <input_file> <working_dir>" << std::endl;
		return 65;
	}

    std::filesystem::current_path(Utils::GetAbsoluteLocation(argv[2]));

    std::string jsonRepo;
    FileHandler::ReadTextFile(Utils::GetAbsoluteLocation(argv[1]), &jsonRepo);
    std::ifstream f(jsonRepo);
    nlohmann::json data;
    try
    {
        data = nlohmann::json::parse(jsonRepo);
    }
    catch (nlohmann::json::parse_error& e)
    {
        std::cerr << "Invalid json file: " << e.what() << "\n";
        std::exit(65);
    }

    for (auto& vars : data["vars"])
    {
        Placeholders::SetPlaceholder(vars["key"], vars["value"]);
    }

    std::string location = data["location"];

    try
    {
        location = Utils::ProcessPlaceholders(location);
    }
    catch (PlaceholderException& e)
    {
        std::cerr << e.what() << std::endl;
		return 65;
    }

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
        downloader.DownloadFile(location);
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
    bool isCompressed = data["output"]["is_compressed"];
    if (isCompressed)
    {
        std::string outputDir = data["output"]["output_dir"].get<std::string>();
        ArchiveOut archive(m_OutputBuffer, outputDir);
        archive.SaveFiles();
    }
    else
    {
        std::string outputFile = data["output"]["output_file"].get<std::string>();
        FileHandler::WriteBinFile(outputFile, m_OutputBuffer);
    }
    if (data.contains("log"))
    {
        std::string log = data["log"]["output_file"];
        try
		{
            std::string logContent = Utils::ProcessPlaceholders(data["log"]["content"]);
            FileHandler::WriteTextFile(log, logContent);
		}
		catch (PlaceholderException& e)
		{
			std::cerr << e.what() << std::endl;
			return 65;
		}
		
    }
    CURLStarter::ShutdownCurl();
    return 0;
}