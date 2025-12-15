#pragma once

#include <memory>
#include <vector>
#include "Chunk.hpp"
#include <curl/curl.h>
#include <unordered_map>
#include <string>

namespace sr2
{
    class Downloader
    {
    public:
        Downloader();
        ~Downloader();        

        bool DownloadFile(std::string_view url);
        void PackMemoryChunks(std::shared_ptr<sr2::RawBuffer>* out_buffer);

        std::string ReturnMime();
        std::string ReturnMappedMime();
        void ClearChunks();
    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

        std::vector<std::shared_ptr<sr2::Chunk>> m_Response;
        static const std::unordered_map<std::string, std::string> s_MimeExtensionMap;
        std::string m_CurrentMime;

        size_t m_CurrentChunk;
        CURL* m_CurlController;
        CURLcode m_ResponseCode;
    };
}
