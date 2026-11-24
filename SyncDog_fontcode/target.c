#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("ERR socket failed");
		close(sock);
		return 1;
	}
	struct sockaddr_in destiny;
	destiny.sin_family = AF_INET;
	destiny.sin_port = htons(50600);
	char address[16];
	printf("Type server address\n");
	fgets(address, sizeof(address), stdin);
	address[strcspn(address, "\n")] = '\0';
	int in_pt = inet_pton(AF_INET, address, &destiny.sin_addr);
	if(in_pt == 0) {
		fprintf(stderr, "ERR not a valid address, did you type in correctly?");
		close(sock);
		return 1;
	}
	int con = connect(sock, (struct sockaddr *)&destiny, sizeof(destiny));
	if(con == -1) {
		perror("ERR connect failed");
		close(sock);
		return 1;
	}
	char msg[14] = "HELLOSYNCDOG";
	int sd = send(sock, msg, strlen(msg), 0);
	if(sd == -1) {
		perror("ERR send failed");
		return 1;
	}
	return 0;
}
