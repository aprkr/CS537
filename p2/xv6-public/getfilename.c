#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char* argv[]){
    char buf[256];
    int n = strlen(argv[1]);
    int fd = open(argv[1], 0);
    int i = getfilename(fd, buf, n);
    if(i == 0){
        printf(1, "XV6_TEST_OUTPUT Open filename: %s\n", buf);
    }
    else{
        printf(1, "XV6_TEST_OUTPUT Open filename: %i\n", i);
    }
    exit();
}
