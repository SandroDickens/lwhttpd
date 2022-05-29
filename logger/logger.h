#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>

#define LOG_LINE_MAX 512

enum class log_level
{
	FATAL = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4,
	TRACE = 5
};



class logger
{
public:
	explicit logger(log_level lv = log_level::WARN);

	explicit logger(const std::string &log_file, log_level level = log_level::WARN);

	explicit logger(int fd, log_level lv = log_level::WARN);

	[[nodiscard]] int get_log_fd() const
	{
		return this->log_fd;
	}

	void set_log_fd(int fd)
	{
		if (fd != -1)
		{
			this->log_fd = fd;
		}
	}

	void set_log_evel(log_level lv)
	{
		this->level = lv;
	}

	[[nodiscard]] log_level get_log_level() const
	{
		return this->level;
	}

	std::string get_log_prefix()
	{
		return this->log_prefix;
	}

	void set_log_prefix(std::string prefix)
	{
		this->log_prefix = std::move(prefix);
	}

	void log(log_level lv, const std::string &log_str);

	void log_fatal(const std::string &log_str);

	void log_error(const std::string &log_str);

	void log_warn(const std::string &log_str);

	void log_info(const std::string &log_str);

	void log_debug(const std::string &log_str);

	void log_trace(const std::string &log_str);

private:
	log_level level;
	int log_fd;
	std::string log_prefix;
};

#endif //_LOGGER_H
