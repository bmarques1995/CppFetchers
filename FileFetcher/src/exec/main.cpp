#include <iostream>
#include "Downloader.hpp"
#include "CurlStarter.hpp"
#include "FileHandler.hpp"
#include "CertDownloader.hpp"

int main(int argc, char* argv[]) {

    CURLStarter::InitCurl();
#if defined(_WIN32)
    CURLStarter::SetCertificateLocation("C:\\curl_config\\certs\\curl-ca-bundle.crt");
#else
    CURLStarter::SetCertificateLocation("~/curl_config/certs/curl-ca-bundle.crt");
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
		return -1;
	}
    downloader.PackMemoryChunks(&m_OutputBuffer);
    std::string filename = "d3d12_sdk.zip";
    FileHandler::WriteBinFile(filename, m_OutputBuffer);
    CURLStarter::ShutdownCurl();
    return 0;
}