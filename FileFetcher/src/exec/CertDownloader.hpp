#pragma once

#include <string>

class CertDownloader
{
public:
	static void DownloadCert();

private:
#ifdef _WIN32
	static std::string s_CMDScript;
#else
	static std::string s_ShellScript;
#endif // 

};
