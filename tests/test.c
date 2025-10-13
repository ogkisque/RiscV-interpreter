#include <stdint.h>

extern long read( int fd, char* data, long maxlen);
extern long write( int fd, const char* data, long len);
extern __attribute__((noreturn)) void exit( long status);

int main( int argc, char* argv[])
{
    int64_t a = 0x1234567890ABCDEF;
    int64_t b = a << 8;
    return (b == 0x34567890ABCDEF00);
}