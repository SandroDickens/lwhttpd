#ifndef LWHTTPD_UTILS_H
#define LWHTTPD_UTILS_H

/**
 * vscnprintf -  Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * The return value is the number of characters which have been written into the buf not including the trailing '\0'.
 * If size is == 0 the function returns 0.
 * If you're not already dealing with a va_list consider using scnprintf.
 * See the vsnprintf documentation for format string extensions over C99.
 */
size_t vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args);

/**
 * scnprintf - Format a string and place it in a buffer
 * @param buf The buffer to place the result into
 * @param size The size of the buffer, including the trailing null space
 * @param fmt The format string to use @...: Arguments for the format string
 * @param ... variable arguments
 * @return The return value is the number of characters written into buf not including the trailing '\0'.
 * If size is == 0 the function returns 0.
 */
size_t scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...);

unsigned long readline(int fd, char *buf, unsigned long size);

void handle_error(const char *func_name, int line, int error_code, int fd, const char *msg);

#endif //LWHTTPD_UTILS_H
