#ifndef LWHTTPD_MIME_TYPES_H
#define LWHTTPD_MIME_TYPES_H


#include <unordered_map>

class MIMETypes
{
public:
	static std::string getMIMEType(const std::string &fileName);

	static bool isELF(const std::string &fileName);

	MIMETypes(const MIMETypes &obj) = delete;

	MIMETypes &operator=(const MIMETypes &) = delete;

	static MIMETypes &getInstance()
	{
		static MIMETypes instance;
		return instance;
	}

private:
	MIMETypes();
	static std::unordered_map<std::string, std::string> mimeMap;
};


#endif //LWHTTPD_MIME_TYPES_H
