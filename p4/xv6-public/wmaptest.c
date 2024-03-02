#include "types.h"
#include "stat.h"
#include "user.h"
// #include "wmap.h"

int main(int argc, char *argv[]) {
    printf(1, "syscall returns: %d\n", wmap(0x60000000, 8192, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1));
    exit();
}
