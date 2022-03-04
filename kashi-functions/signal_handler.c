#include <stdio.h>
#include <signal.h>

void handler (int signo, siginfo_t *info, void *context){
    printf ("Oyee.... Don't Interrupt !!\n");
}

void signal_handler (int signal, void (*func) (int, siginfo_t*, void*)) {
    struct sigaction act = {0};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = func;
    sigaction (signal, &act, NULL);
}

int main () {
    signal_handler (SIGINT, handler);

    while (1);

    return 0;
}