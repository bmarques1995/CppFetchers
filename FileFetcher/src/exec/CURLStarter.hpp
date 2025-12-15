#pragma once

#include <cstddef>
#include <string>
#include <cstdlib>


namespace sr2
{
    class CURLStarter
    {
    public:
        static void InitCurl();
        static void ShutdownCurl();

        static void SetCurlChunkSize(size_t size);
        static void SetCertificateLocation(std::string location);
        
        static size_t GetCurlChunkSize();
        static std::string GetCertificateLocation();
    private:
        static std::string s_CertificateLocation;
        static size_t s_ChunkSize;
    };
}