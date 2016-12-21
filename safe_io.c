#include <stdio.h>
#include <pthread.h>
#include "safe_io.h"

static pthread_mutex_t io_mx = PTHREAD_MUTEX_INITIALIZER;

void vprint(const char *type, const char *msg, va_list args)
{
	pthread_mutex_lock(&io_mx);
	fprintf(stderr, "%s: ", type);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	fflush(stderr);
	pthread_mutex_unlock(&io_mx);
}

void print(const char *type, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprint(type, msg, args);
	va_end(args);
}

void info(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprint("Info", msg, args);
	va_end(args);
}

void error(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprint("Error", msg, args);
	va_end(args);
}
