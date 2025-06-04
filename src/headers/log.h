#include <windows.h>

#ifndef log
#define log

int WriteToLogStr(char *content);

int WriteToLogBits(unsigned char *content, size_t len);

#endif