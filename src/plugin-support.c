/* src/plugin-support.c */
#include "plugin-support.h"

#include <obs.h>
#include <stdarg.h>
#include <stdio.h>

void obs_log(int log_level, const char *format, ...)
{
	size_t length = 4 + strlen(PLUGIN_NAME) + strlen(format) + 1;
	char *with_prefix = (char *)bmalloc(length);
	snprintf(with_prefix, length, "[%s] %s", PLUGIN_NAME, format);

	va_list args;
	va_start(args, format);
	blogva(log_level, with_prefix, args);
	va_end(args);

	bfree(with_prefix);
}
