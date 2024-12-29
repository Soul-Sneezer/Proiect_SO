#ifndef HEADER_RW_FUNC
#define HEADER_RW_FUNC

#include <sys/types.h>

ssize_t readn(int fd, void* buffer, size_t n);
ssize_t writen(int fd, const void* buffer, size_t n);

#endif
