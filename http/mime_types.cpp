#include <linux/elf.h>
#include <string>
#include <cstring>
#include <iostream>
#include "mime_types.h"

std::unordered_map<std::string, std::string> MIMETypes::mimeMap;

MIMETypes::MIMETypes()
{
	/* text/xxxx */
	std::string type = "text";
	{
		std::string plainType[] = {"txt", "ini", "conf", "log", "c", "cpp", "h", "hpp", "java", "php", "Makefile", "v"};
		for (const std::string &tmp:plainType)
		{
			mimeMap.insert({tmp, type + "/" + "plain"});
		}
		mimeMap.insert({"css", type + "/" + "css"});
		mimeMap.insert({"html", type + "/" + "html"});
		mimeMap.insert({"htm", type + "/" + "html"});
		mimeMap.insert({"js", type + "/" + "javascript"});
	}
	/* image/xxx */
	type = "image";
	{
		mimeMap.insert({"gif", type + "/" + "gif"});
		mimeMap.insert({"png", type + "/" + "png"});
		mimeMap.insert({"jpeg", type + "/" + "jpeg"});
		mimeMap.insert({"jpg", type + "/" + "jpg"});
		mimeMap.insert({"bmp", type + "/" + "bmp"});
		mimeMap.insert({"ico", type + "/" + "x-icon"});
		mimeMap.insert({"svg", type + "/" + "svg+xml"});
	}
	/* audio/xxx */
	type = "audio";
	{
		mimeMap.insert({"wav", type + "/" + "wav"});
		mimeMap.insert({"ogg", type + "/" + "ogg"});
	}

	/* video/xxx */
	type = "video";
	{
		mimeMap.insert({"webm", type + "/" + "webm"});
	}
	/* application/xxx */
	type = "application";
	{
		mimeMap.insert({"xml", type + "/" + "xml"});
		mimeMap.insert({"json", type + "/" + "json"});
		mimeMap.insert({"elf", type + "/" + "octet-stream"});
	}
}

std::string MIMETypes::getMIMEType(const std::string &fileName)
{
	if (isELF(fileName))
	{
		return mimeMap.at("elf");
	}
	else
	{
		size_t pos = fileName.find_last_of('.');
		std::string suffix = fileName.substr(pos + 1);
		if (mimeMap.contains(suffix))
		{
			return mimeMap.at(suffix);
		}
		else
		{
			return "application/octet-stream";
		}
	}
}

bool MIMETypes::isELF(const std::string &fileName)
{
	bool is_elf = false;
	elf64_hdr elf_header{};
	memset(&elf_header, 0, sizeof(elf_header));

	FILE *fw = fopen64(fileName.c_str(), "rb");
	if (fw != nullptr)
	{
		if (fread(&elf_header, sizeof(elf_header), 1, fw) > 0)
		{
			if ((elf_header.e_ident[EI_MAG0] == 0x7f) && (elf_header.e_ident[EI_MAG1] == 'E') &&
			    (elf_header.e_ident[EI_MAG2] == 'L') && (elf_header.e_ident[EI_MAG3] == 'F'))
			{
				is_elf = true;
			}
		}
		fclose(fw);
	}
	else
	{
		std::cerr << "Can not open the specified file" << std::endl;
		throw std::exception();
	}
	return is_elf;
}
