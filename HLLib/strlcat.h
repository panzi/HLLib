#ifndef STRLCAT_H
#define STRLCAT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlcat(char *dst, const char *src, size_t siz);

#ifdef __cplusplus
}
#endif

#endif
