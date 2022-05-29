#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include <cstring>

#include "logger.h"

static std::string to_string(log_level level)
{
	std::string str;
	switch (level)
	{
		case log_level::FATAL:
			str = "FATAL";
			break;
		case log_level::ERROR:
			str = "ERROR";
			break;
		case log_level::WARN:
			str = "WARN";
			break;
		case log_level::INFO:
			str = "INFO";
			break;
		case log_level::DEBUG:
			str = "DEBUG";
			break;
		case log_level::TRACE:
			str = "TRACE";
			break;
	}
	return str;
}

logger::logger(log_level lv)
{
	this->level = lv;
	this->log_fd = STDOUT_FILENO;
	this->log_prefix = "";
}

logger::logger(const std::string &log_file, log_level lv)
{
	this->level = lv;
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	int fd = open(log_file.c_str(), O_RDWR|O_APPEND|O_CLOEXEC|O_CREAT, mode);
	if (fd != -1)
	{
		this->log_fd = fd;
	}
	else
	{
		std::cerr << "can not open or create log file " << log_file << ", Use STDOUT instead: " << strerror(errno)
		          << "(" << errno << ")"
		          << std::endl;
		this->log_fd = STDOUT_FILENO;
	}
	this->log_prefix = "";
}

logger::logger(int fd, log_level lv)
{
	this->level = lv;
	if (fd != -1)
	{
		this->log_fd = fd;
	}
	else
	{
		std::cerr << "invalid log fd " << fd << std::endl;
	}
	this->log_prefix = "";
}

void logger::log(log_level lv, const std::string &log_str)
{
	if (lv <= this->level)
	{
		time_t time_now;
		time(&time_now);
		tm *tm_now = localtime(&time_now);
		std::string log_content;
		char time_str[64];
		time_str[0] = '\0';
		snprintf(time_str, sizeof(time_str), "%d/%d/%d %02d:%02d:%02d %s, ", 1900 + tm_now->tm_year, tm_now->tm_mon + 1,
		         tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, tm_now->tm_zone);
		log_content += time_str;
		log_content += "[" + to_string(lv) + "], ";
		if (!this->log_prefix.empty())
		{
			log_content += '[' + this->log_prefix + "] ";
		}
		log_content += log_str + '\n';
		write(this->log_fd, log_content.c_str(), log_content.length());
	}
}

void logger::log_fatal(const std::string &log_str)
{
	this->log(log_level::FATAL, log_str);
}

void logger::log_error(const std::string &log_str)
{
	this->log(log_level::ERROR, log_str);
}

void logger::log_warn(const std::string &log_str)
{
	this->log(log_level::WARN, log_str);
}

void logger::log_info(const std::string &log_str)
{
	this->log(log_level::INFO, log_str);
}

void logger::log_debug(const std::string &log_str)
{
	this->log(log_level::DEBUG, log_str);
}

void logger::log_trace(const std::string &log_str)
{
	this->log(log_level::TRACE, log_str);
}
