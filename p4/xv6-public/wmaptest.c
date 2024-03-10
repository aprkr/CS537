#include "types.h"
#include "stat.h"
#include "user.h"
// #include "wmap.h"

int main(int argc, char *argv[]) {
    printf(1, "syscall returns: %d\n", wmap(0x60000000, 8192, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1));
    
    struct wmapinfo idk;
    idk.total_mmaps = -1;
    getwmapinfo(&idk);
    printf(1, "%d\n%d\n", idk.addr[0], idk.total_mmaps);
    // wmapinfo(idk);
    // printf(1, "%d\n", idk[0]->addr);
    // printf(1, "%d\n", idk[0]->length);
    printf(1, "%d\n", wunmap(0x60000000));
    exit();
}
