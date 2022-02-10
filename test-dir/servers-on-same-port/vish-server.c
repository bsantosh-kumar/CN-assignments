#include <stdio.h>
#include <string.h> // strlen
#include <sys/socket.h>
#include <arpa/inet.h> // inet_pton
#include <unistd.h>
int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in saddr; // server address
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8080);
    saddr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    // Forcefully attaching socket to the port, so that bind already in use error doesnt occur
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        return 1;
    }

    int bind_status = bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (bind_status == -1)
    {
        perror("bind");
        return 1;
    }

    int listen_status = listen(sfd, 5);
    if (listen_status == -1)
    {
        perror("listen");
        return 1;
    }

    struct sockaddr_in caddr;
    socklen_t caddr_len = sizeof(caddr);
    int nsfd = accept(sfd, (struct sockaddr *)&caddr, &caddr_len);
    if (nsfd == -1)
    {
        perror("accept");
        return 1;
    }

    char message[1024];
    recv(nsfd, message, sizeof(message), 0);
    printf("Recieved %s\n", message);

    char *reply = "hello client\n";
    send(nsfd, reply, strlen(reply), 0);

    close(nsfd);
    close(sfd);
    return 0;
}