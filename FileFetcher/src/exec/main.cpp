#include <iostream>
#include "Downloader.hpp"
#include "CurlStarter.hpp"
#include "CertDownloader.hpp"
#include "FileHandler.hpp"

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
    std::string filename = "d3d12_sdk.zip";
    FileHandler::WriteBinFile(filename, m_OutputBuffer);
    CURLStarter::ShutdownCurl();
    return 0;
}