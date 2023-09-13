#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int ret = open("/dev/mydevice", O_RDONLY);
    if (ret == -1) {
        printf("Opening was not possible!\n");
        return -1;
    }
    printf("Opening was succesfull!\n");
    close(ret);
    return 0;
}

