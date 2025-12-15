#include "CURLStarter.hpp"
#include <curl/curl.h>

size_t sr2::CURLStarter::s_ChunkSize = 1024;
std::string sr2::CURLStarter::s_CertificateLocation = "";

void sr2::CURLStarter::InitCurl()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void sr2::CURLStarter::ShutdownCurl()
{
    curl_global_cleanup();
}

void sr2::CURLStarter::SetCurlChunkSize(size_t size)
{
    s_ChunkSize = size;
}

void sr2::CURLStarter::SetCertificateLocation(std::string location)
{
    s_CertificateLocation = location;
}

size_t sr2::CURLStarter::GetCurlChunkSize()
{
    return s_ChunkSize;
}

std::string sr2::CURLStarter::GetCertificateLocation()
{
    return s_CertificateLocation;
}
