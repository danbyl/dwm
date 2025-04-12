/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void
die(const char *fmt, ...)
{
	va_list ap;
	int saved_errno;

	saved_errno = errno;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':')
		fprintf(stderr, " %s", strerror(saved_errno));
	fputc('\n', stderr);

	exit(1);
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

void
log_msg(const char *func, const char *fmt, ...)
{
	FILE *fp = fopen("/tmp/dwm.log", "a");
	if (!fp)
		return;

	va_list ap;
	va_start(ap, fmt);
	fprintf(fp, "%s: ", func);
	vfprintf(fp, fmt, ap);
	fputc('\n', fp);
	va_end(ap);
	fclose(fp);
}
