#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_DBL_PRECISION 15
#define DEC_BASE 10
#define HEX_BASE 16

char *_HEX_DIGITS = "0123456789ABCDEF";

static bool __print(const char *data, size_t length, char *buffer) 
{
    if (buffer == NULL) {
        const unsigned char* bytes = (const unsigned char*) data;
	    for (size_t i = 0; i < length; i++)
		    if (putchar(bytes[i]) == EOF)
			    return false;
	    return true;
    }

    memcpy(buffer, data, length);
	return true;
}

static void __reverse(void *buffer, size_t len)
{
    uint8_t tmp[len];
    memrcpy((void *)tmp, buffer, len);
    memcpy(buffer, tmp, len);
}

static char * __strcpy(char *dest, char *src)
{
    return memcpy(dest, src, strlen(src)+1);
}

static size_t __uint_to_string(uint64_t value, char *buffer)
{
    size_t index = 0;
    while(value >= 10)
    {
        uint8_t remainder = value % DEC_BASE;
        value /= 10;
        buffer[index++] = remainder + '0';
    }

    size_t remainder = value % DEC_BASE;
    buffer[index++] = remainder + '0';
    __reverse(buffer, index);
    buffer[index] = 0;
    return index;
}

static size_t __int_to_string(int64_t value, char *buffer)
{
    if (value >= 0) return __uint_to_string((uint64_t)value, buffer);

    value *= -1;
    size_t len = __uint_to_string((uint64_t)value, buffer+1);
    buffer[0] = '-';
    return len;
}

static size_t __uint_to_hex(uint64_t value, char *buffer)
{
    size_t index = 2;
    while (value >= HEX_BASE)
    {
        int remainder = value % HEX_BASE;
        value /= HEX_BASE;
        buffer[index++] = _HEX_DIGITS[remainder];
    }

    if (value > 0) buffer[index++] = _HEX_DIGITS[value];

    __reverse(buffer+2, index-2);
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[index] = 0;
    return index;
}

static size_t __int_to_hex(int64_t value, char *buffer)
{
    return __uint_to_hex((uint64_t)value, buffer);
}

static size_t __double_to_string(double value, uint8_t precision, char *buffer)
{
    if (precision > MAX_DBL_PRECISION) precision = MAX_DBL_PRECISION;
    uint64_t multiplier = 1 ;
    for(int i=0; i < precision; i++) multiplier *= 10;

    size_t len = 0;
    char whole_buffer[128];
    __int_to_string((int64_t)value, whole_buffer);
    size_t whole_len = strlen(whole_buffer);
    __strcpy(buffer, whole_buffer);
    buffer[whole_len] = '.';
    len += whole_len + 1;

    if (value < 0) value *= -1;
    double decimalValue = (value - (uint64_t)value) * multiplier;
    char decimal_buffer[128];
    __uint_to_string((uint64_t)decimalValue, decimal_buffer);
    size_t decimal_len = strlen(decimal_buffer);
    memcpy(buffer+len, decimal_buffer, decimal_len);
    len += decimal_len;

    buffer[len] = 0;
    return len;
}

static int __printf(char *buffer, const char* restrict format, va_list parameters) 
{
	int written = 0;
    char *buf = buffer;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;
        if (buf)
            buf = buffer + written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!__print(format, amount, buf))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!__print(&c, sizeof(c), buf))
				return -1;
			written++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!__print(str, len, buf))
				return -1;
			written += len;
        } else if (*format == 'i') {
            format++;
            const int64_t val = (int64_t) va_arg(parameters, int64_t);
            if (!maxrem) {
                return -1;
            }
            char tmpbuf[128];
            size_t len = __int_to_string(val, tmpbuf);
            if (!__print(tmpbuf, len, buf))
                return -1;
            written += len;
        } else if (*format == 'u') {
            format++;
            const uint64_t val = (uint64_t) va_arg(parameters, uint64_t);
            if (!maxrem) {
                return -1;
            }
            char tmpbuf[128];
            size_t len = __uint_to_string(val, tmpbuf);
            if (!__print(tmpbuf, len, buf))
                return -1;
            written += len;
        } else if (*format == 'd') {
            format++;
            const double val = (double) va_arg(parameters, double);
            if (!maxrem) {
                return -1;
            }
            char tmpbuf[128];
            size_t len = __double_to_string(val, 2, tmpbuf);
            if (!__print(tmpbuf, len, buf))
                return -1;
            written += len;
        } else if (*format == 'x') {
            format++;
            const uint64_t val = (uint64_t) va_arg(parameters, uint64_t);
            if (!maxrem) {
                return -1;
            }
            char tmpbuf[20];
            size_t len = __uint_to_hex(val, tmpbuf);
            if (!__print(tmpbuf, len, buf))
                return -1;
            written += len;
		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!__print(format, len, buf))
				return -1;
			written += len;
			format += len;
		}
	}

	return written;
}

int printf(const char* restrict format, ...) 
{
    va_list parameters;
	va_start(parameters, format);
    int written = __printf(NULL, format, parameters);
    va_end(parameters);
    return written;
}

int sprintf(char *buffer, const char* restrict format, ...)
{
    va_list parameters;
	va_start(parameters, format);
    int written = __printf(buffer, format, parameters);
    va_end(parameters);
    buffer[written] = 0;
    return written;
}