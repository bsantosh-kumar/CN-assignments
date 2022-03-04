#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080

int set_server (int port) {
	int sfd;
	struct sockaddr_in serv_addr;

	if ((sfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1){
		printf ("Unable to create socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
		printf ("Invalid network address or address family\n");
		return 0;
	}

	if (bind (sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		printf ("Error in bind\n");
		return -1;
	}

	return sfd;
}

int main(){
    char buff[50] = {0};
	char msg[50] = {0};

    int sfd;
	struct sockaddr_in cli_addr;
	int cli_addrlen = sizeof(cli_addr);

	sfd = set_server (PORT);

    recvfrom (sfd, buff, sizeof(buff), 0, (struct sockaddr *)&cli_addr, (socklen_t*)&cli_addrlen);
    printf ("Message Received : %s\n", buff);

    fgets (msg, sizeof(msg), stdin);
    sendto (sfd, msg, strlen(msg), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));

	return 0;
}
