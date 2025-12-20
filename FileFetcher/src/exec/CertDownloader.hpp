#pragma once

#include <string>

class CertDownloader
{
public:
	static void DownloadCert();
	static void AssertCertHash(std::string_view certLocation);

private:
	static std::string GetExpectedHash(std::string_view certLocation);
	static std::string GetCertHash(std::string_view certLocation);
	static std::string Digest(const std::string& str);

#ifdef _WIN32
	static std::string s_CMDScript;
#else
	static std::string s_ShellScript;
#endif // 

};
