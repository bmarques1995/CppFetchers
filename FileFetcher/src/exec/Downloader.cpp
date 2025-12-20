#include "Downloader.hpp"
#include <stdexcept>
#include <cstring>
#include <iostream>
#include "CURLStarter.hpp"
#include <exception>

struct DownloaderInfo{
    size_t* CurrentChunk;
    std::vector<std::shared_ptr<Chunk>>* Chunks;
    DownloaderInfo() : CurrentChunk(nullptr), Chunks(nullptr)
    {}
};

const std::unordered_map<std::string, std::string> Downloader::s_MimeExtensionMap = {
    {"application/json", "json"},
    {"application/json; charset=utf-8", "json"},
    {"text/html", "html"},
    {"text/plain", "txt"},
    {"image/png", "png"},
    {"image/jpeg", "jpg"},
};
        


Downloader::Downloader() :
	m_CurrentChunk(0),
    m_CurrentMime(""),
	m_ResponseCode(CURLE_OK)
{
    m_CurlController = curl_easy_init();
    if(!m_CurlController)
        throw std::runtime_error("Failed to initialize downloader");
}

Downloader::~Downloader()
{
    curl_easy_cleanup(m_CurlController);
}

bool Downloader::DownloadFile(std::string_view url)
{
    DownloaderInfo info;
    info.CurrentChunk = &m_CurrentChunk;
    info.Chunks = &m_Response;

    // Set URL
    curl_easy_setopt(m_CurlController, CURLOPT_URL, url.data());

    SetSSL();

    // Set write callback
    curl_easy_setopt(m_CurlController, CURLOPT_WRITEFUNCTION, Downloader::WriteCallback);

    // Set pointer to store the response
    curl_easy_setopt(m_CurlController, CURLOPT_WRITEDATA, &info);

    // Perform the request
    m_ResponseCode = curl_easy_perform(m_CurlController);

    if (m_ResponseCode != CURLE_OK && m_ResponseCode != CURLE_SSL_CACERT_BADFILE) {
        std::cerr << "curl_easy_perform() failed: "
            << curl_easy_strerror(m_ResponseCode) << std::endl;
    }

    if (m_ResponseCode == CURLE_SSL_CACERT_BADFILE)
    {
		throw InvalidCertLocation();
		return false;
    }

    // Check HTTP status code
    long http_code = 0;
    curl_easy_getinfo(m_CurlController, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "Unexpected HTTP status: " << http_code << "\n";;
        return false;
    }

    // Check MIME type
    curl_header *type;
    CURLHcode h;
    h = curl_easy_header(m_CurlController, "Content-Type", 0, CURLH_HEADER, -1, &type);
    m_CurrentMime = type->value;
    return true;
}

void Downloader::PackMemoryChunks(std::shared_ptr<RawBuffer>* out_buffer)
{
    if (m_Response.empty() || out_buffer == nullptr)
        return;

    size_t total_size = 0;
    for (const auto& chunk : m_Response)
    {
        total_size += chunk->GetSize();
    }

    uint8_t* combined_data = new uint8_t[total_size];
    size_t offset = 0;
    for (const auto& chunk : m_Response)
    {
        memcpy(combined_data + offset, chunk->GetData(), chunk->GetSize());
        offset += chunk->GetSize();
    }

    (*out_buffer)->RecreateBuffer(combined_data, total_size);
    delete[] combined_data;
}

std::string Downloader::ReturnMime()
{
    return m_CurrentMime;
}

std::string Downloader::ReturnMappedMime()
{
    auto it = s_MimeExtensionMap.find(m_CurrentMime);
    if (it != s_MimeExtensionMap.end()) {
        return it->second;
    }
    return "";
}

void Downloader::ClearChunks()
{
    m_Response.clear();
    m_CurrentChunk = 0;
}

size_t Downloader::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t bufferSize = size * nmemb;
    std::shared_ptr<Chunk> chunk;
    DownloaderInfo* buffer = static_cast<DownloaderInfo*>(userp);
    chunk.reset(new Chunk((*buffer->CurrentChunk)));
    chunk->RecreateBuffer((uint8_t*)contents, bufferSize);
    buffer->Chunks->push_back(chunk);
    (*(buffer->CurrentChunk))++;
    return bufferSize;
}

void Downloader::SetSSL()
{
    curl_easy_setopt(m_CurlController, CURLOPT_CAINFO, CURLStarter::GetCertificateLocation().c_str());
}
