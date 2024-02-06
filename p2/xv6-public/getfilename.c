#include "defs.h"
#include "string.h"
#include "types.h"

int main(int argc, char* argv[]){
    char buf[256];
    int n = strlen(argv[0]);
    syscall();
    int fd = sys_open(argv[0]);
    int i = sys_getfilename(fd, buf, n);
    printf("XV6_TEST_OUTPUT Open filename: %i", i);
}
