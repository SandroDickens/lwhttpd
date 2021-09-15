#include <cstdarg>
#include <cstdio>
#include <sys/socket.h>

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int i = vsnprintf(buf, size, fmt, args);
	ssize_t ssize = size;
	return (i >= ssize) ? (ssize - 1) : i;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
	ssize_t ssize = size;
	va_list args;
	int i;
	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return (i >= ssize) ? (ssize - 1) : i;
}

unsigned long readline(int fd, char *buf, unsigned long size)
{
	unsigned long read_size = 0;
	char ch = '\0';
	int n = 0;
	while ((read_size < size - 1) && (ch != '\n'))
	{
		n = recv(fd, &ch, 1, 0);
		if (n > 0)
		{
			if (ch == '\r')
			{
				n = recv(fd, &ch, 1, MSG_PEEK);
				if ((n > 0) && (ch == '\n'))
				{
					recv(fd, &ch, 1, 0);
				}
				else
				{
					ch = '\n';
				}
			}
			buf[read_size++] = ch;
		}
		else
		{
			ch = '\n';
		}
	}
	buf[read_size] = '\0';
	return read_size;
}
