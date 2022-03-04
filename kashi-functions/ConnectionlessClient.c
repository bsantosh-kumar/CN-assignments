#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080

int set_client (int port, struct sockaddr_in *serv_addr) {
	int sfd;

    if ((sfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1){
		perror ("Error in socket");
		exit (0);
	}

	(*serv_addr).sin_family = AF_INET;
	(*serv_addr).sin_port = htons (port);
	if (inet_pton (AF_INET, "127.0.0.1", &(*serv_addr).sin_addr) <= 0){
		perror ("Error in network address");
		exit (0);
	}	

	return sfd;
}

int main(){
	char msg[50] = {0};
	char buff[50] = {0};

    int sfd;
	struct sockaddr_in serv_addr;
	int addrlen = sizeof(serv_addr);

    sfd = set_client (PORT, &serv_addr);

    fgets (msg, sizeof(msg), stdin);
    sendto (sfd, msg, strlen(msg), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    recvfrom (sfd, buff, sizeof(buff), 0, (struct sockaddr *)&serv_addr, (socklen_t*)&addrlen);
    printf ("Message Received : %s\n", buff);

	return 0;
}
