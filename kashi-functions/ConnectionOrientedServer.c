#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080

int set_server (int port) {
	int sfd;
	struct sockaddr_in serv_addr;
	int serv_addrlen = sizeof(serv_addr);
	int opt = 1;

	if ((sfd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
		perror ("Error in socket");
		exit (0);
	}

	if (setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1 ){
        perror("Error in setsockopt");
        exit(0);
    }

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (port);
	
	if (inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
		perror ("Error in network address");
		exit (0);
	}

	if (bind (sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		printf ("Error in bind\n");
		return -1;
	}

	if (getsockname (sfd, (struct sockaddr *)&serv_addr, (socklen_t*)&serv_addrlen) == -1){
		printf ("Unable to get socket\n");
		return 0;
	}

	printf ("Local IP Addresss : %s\n", inet_ntoa (serv_addr.sin_addr));
	printf ("Local Port : %d\n\n", ntohs (serv_addr.sin_port));


	if (listen (sfd, 3) == -1){
		printf ("Error in listen\n");
		return -1;
	}

	return sfd;
}

int main(){
    int sfd, nsfd, valrecv;
	struct sockaddr_in cli_addr;
	int cli_addrlen = sizeof(cli_addr);

	sfd = set_server (PORT);

	while (1){
		char buff[50] = {0};

		if ((nsfd = accept (sfd, (struct sockaddr *)&cli_addr, (socklen_t*)&cli_addrlen)) == -1){
			printf ("Error in accept\n");
			return -1;
		}

		if (getpeername (nsfd, (struct sockaddr *)&cli_addr, (socklen_t*)&cli_addrlen) == -1){
			printf ("Unable to get socket\n");
			return 0;
		}

		printf ("Foreign IP Addresss : %s\n", inet_ntoa (cli_addr.sin_addr));
		printf ("Foreign Port : %d\n\n", ntohs (cli_addr.sin_port));

		recv (nsfd , buff, sizeof(buff), 0);
		printf ("Message Received : %s\n",buff);

		close (nsfd);

		printf ("Client was terminated\n\n\n");

		if (strcmp ("exit\n", buff) == 0){
			printf ("Server Terminated\n");
			break;
		}
	}

	return 0;
}
