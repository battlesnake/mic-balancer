#pragma once
#include <stdarg.h>

void vprint(const char *type, const char *msg, va_list args);
void print(const char *type, const char *msg, ...);
void info(const char *msg, ...);
void error(const char *msg, ...);
