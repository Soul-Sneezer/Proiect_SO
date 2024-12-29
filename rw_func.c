#include <unistd.h>
#include <errno.h>
#include "rw_func.h"

ssize_t readn(int fd, void* buffer, size_t n)
{
    size_t bytesRead = n;
    size_t totBytesRead = 0;
    char* buf = buffer;

    for (totBytesRead = 0; totBytesRead < n;)
    {
        if((bytesRead = read(fd, buf, n - totBytesRead)) == -1) {
            if (errno == EINTR) // interrupted
                continue;
            else
                return -1;
        } else if(bytesRead == 0) { // end of file
            return totBytesRead;
        }
        
        totBytesRead += bytesRead;
        buf += bytesRead;
    }

    return totBytesRead;
}

ssize_t writen(int fd, const void* buffer, size_t n)
{
    size_t bytesWritten = n;
    size_t totBytesWritten = 0;
    const char* buf = buffer;

    for (totBytesWritten = 0; totBytesWritten < n;)
    {
        if((bytesWritten = write(fd, buf, n - totBytesWritten)) <= 0) {
            if (bytesWritten == -1 && errno == EINTR) // interrupted
                continue;
            else
                return -1;
        } 
        
        totBytesWritten += bytesWritten;
        buf += bytesWritten;
    }

    return totBytesWritten;

}
