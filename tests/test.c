#include <stdint.h>

extern long read( int fd, char* data, long maxlen);
extern long write( int fd, const char* data, long len);
extern __attribute__((noreturn)) void exit( long status);

int main( int argc, char* argv[])
{
    return (*argv[1] - '0');
}