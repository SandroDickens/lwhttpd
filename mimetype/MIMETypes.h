#ifndef TINYHTTPD_MIMETYPES_H
#define TINYHTTPD_MIMETYPES_H


#include <unordered_map>

class MIMETypes
{
public:
	std::string getMIMEType(const std::string &fileName);

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

};


#endif //TINYHTTPD_MIMETYPES_H
