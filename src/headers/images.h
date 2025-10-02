#include "generalvars.h"

#ifndef images
#define images

HBITMAP LoadPNGAsBmp(char *file_name);

int CompareBytes(unsigned char *bytes, unsigned char *target, size_t len);

#endif