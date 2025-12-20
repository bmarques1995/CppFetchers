#include "CertDownloader.hpp"
#include "FileHandler.hpp"
#include "ProcessDispatcher.hpp"

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

CURL_CA_PATH_INNER="~/curl_config/certs"

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
