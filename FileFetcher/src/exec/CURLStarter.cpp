#include "CURLStarter.hpp"
#include <curl/curl.h>

size_t CURLStarter::s_ChunkSize = 1024;
std::string CURLStarter::s_CertificateLocation = "";

void CURLStarter::InitCurl()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void CURLStarter::ShutdownCurl()
{
    curl_global_cleanup();
}

void CURLStarter::SetCurlChunkSize(size_t size)
{
    s_ChunkSize = size;
}

void CURLStarter::SetCertificateLocation(std::string location)
{
    s_CertificateLocation = location;
}

size_t CURLStarter::GetCurlChunkSize()
{
    return s_ChunkSize;
}

std::string CURLStarter::GetCertificateLocation()
{
    return s_CertificateLocation;
}

const char* InvalidCertLocation::what() const noexcept
{
    return "Certificate file not found, a new one will be downloaded\njust run the program again\n";
}
