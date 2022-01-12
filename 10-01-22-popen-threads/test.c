// server
#include <errno.h>
#include <fcntl.h> /* Definition of AT_* constants */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// https://unix.stackexchange.com/questions/564098/named-pipe-buffer-after-process-end

int main() {
    if (mkfifo("channel", 0444) == -1) {
        printf("mkfifo error\n");
        perror(strerror(errno));
        return 1;
    }
    int wffd = open("channel", O_WRONLY);
    if (wffd == -1) {
        printf("open error\n");
        return 1;
    }
    write(wffd, "hello", 5);

    int rffd = open("channel", O_RDONLY);
    if (rffd == -1) {
        printf("open error\n");
        return 1;
    }
    char buf[10];
    read(rffd, buf, 10);
    printf("%s\n", buf);
}