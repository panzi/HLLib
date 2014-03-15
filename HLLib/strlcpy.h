#ifndef STRLCPY_H
#define STRLCPY_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlcpy(char *dst, const char *src, size_t siz);

#ifdef __cplusplus
}
#endif

#endif
