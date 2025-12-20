#include "CertDownloader.hpp"
#include "CurlStarter.hpp"
#include "FileHandler.hpp"
#include "ProcessDispatcher.hpp"
#include <sstream>
#include <iostream>
#include <openssl/evp.h> // Include the OpenSSL SHA header
#include <iomanip>

#ifdef _WIN32
std::string CertDownloader::s_CMDScript = R"(@echo off
set CURL_CA_PATH_INNER=C:\curl_config\certs

if not exist "%CURL_CA_PATH_INNER%" (
    mkdir "%CURL_CA_PATH_INNER%"
)

set CA_FILE=%CURL_CA_PATH_INNER%\curl-ca-bundle.crt
set HASH_FILE=%CA_FILE%.sha256

echo %CA_FILE%

curl https://curl.se/ca/cacert.pem.sha256 --output "%HASH_FILE%"
curl https://curl.se/ca/cacert.pem --output "%CA_FILE%"

if errorlevel 1 (
    echo Failed to download CA bundle
    exit /b 1
)

echo CA bundle installed at %CA_FILE%
)";
#else
std::string CertDownloader::s_ShellScript=R"(#!/usr/bin/env sh

CURL_CA_PATH_INNER="$HOME/curl_config/certs"

# Ensure directory exists
if [ ! -d "$CURL_CA_PATH_INNER" ]; then
    mkdir -p "$CURL_CA_PATH_INNER" || {
        echo "Failed to create directory: $CURL_CA_PATH_INNER" >&2
        exit 1
    }
fi

CA_FILE="$CURL_CA_PATH_INNER/curl-ca-bundle.crt"
HASH_FILE="$CA_FILE.sha256"

echo "$CA_FILE"

# Download hash and CA bundle
curl -fL https://curl.se/ca/cacert.pem.sha256 -o "$HASH_FILE" &&
curl -fL https://curl.se/ca/cacert.pem -o "$CA_FILE"

if [ $? -ne 0 ]; then
    echo "Failed to download CA bundle" >&2
    exit 1
fi

echo "CA bundle installed at $CA_FILE"
)";
#endif

struct EVP_MD_CTX_Delete {
    void operator()(EVP_MD_CTX* p) const { EVP_MD_CTX_free(p); }
};
using EVP_MD_CTX_ptr = std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Delete>;

void CertDownloader::DownloadCert()
{
#ifdef _WIN32
	FileHandler::WriteTextFile("install_ca_shell.bat", s_CMDScript);
	ProcessDispatcher::ExecuteCommand("cmd.exe", { "/C", "install_ca_shell.bat" });
    FileHandler::DeleteFileAt("./install_ca_shell.bat");
#else
    FileHandler::WriteTextFile("install_ca_shell.sh", s_ShellScript);
    ProcessDispatcher::ExecuteCommand("zsh", { "./install_ca_shell.sh" });
    ProcessDispatcher::ExecuteCommand("bash", { "./install_ca_shell.sh" });
	FileHandler::DeleteFileAt("install_ca_shell.sh");
#endif
    
}

void CertDownloader::AssertCertHash(std::string_view certLocation)
{
    std::string expectedHash = GetExpectedHash(certLocation);
    std::string certHash = GetCertHash(certLocation);
    if(expectedHash.compare(certHash) != 0)
    {
        throw InvalidCert();
	}
}

std::string CertDownloader::GetExpectedHash(std::string_view certLocation)
{
    std::string hashFile;
    FileHandler::ReadTextFile(std::string(certLocation) + "curl-ca-bundle.crt.sha256", &hashFile);
    std::istringstream hashStream(hashFile);
    std::string expectedHash;
    hashStream >> expectedHash;
    return expectedHash;
}

std::string CertDownloader::GetCertHash(std::string_view certLocation)
{
    std::string certFile;
    FileHandler::ReadTextFile(std::string(certLocation) + "curl-ca-bundle.crt", &certFile);
    return Digest(certFile);
}

std::string CertDownloader::Digest(const std::string& data)
{
    EVP_MD_CTX_ptr context(EVP_MD_CTX_new());
    if (!context) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (1 != EVP_DigestInit_ex(context.get(), EVP_sha256(), NULL)) {
        throw std::runtime_error("Failed to initialize digest");
    }

    if (1 != EVP_DigestUpdate(context.get(), data.c_str(), data.length())) {
        throw std::runtime_error("Failed to update digest");
    }

    std::vector<unsigned char> digest(EVP_MD_size(EVP_sha256()));
    unsigned int md_len;
    if (1 != EVP_DigestFinal_ex(context.get(), digest.data(), &md_len)) {
        throw std::runtime_error("Failed to finalize digest");
    }

    std::stringstream ss;
    for (int i = 0; i < EVP_MD_size(EVP_sha256()); i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}
