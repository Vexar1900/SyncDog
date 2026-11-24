#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void clearbuffer() {
	int c;
	while((c = getchar()) != '\n' && c != EOF);
}

typedef struct {
	char path[512];
	char name[256];
	size_t size;
}mp3;

typedef struct {
	mp3 *stored;
	size_t count;
	size_t capacity;
}library;

int createData() {
	if(mkdir("SyncDogData", 0755) == -1) {
		perror("ERR creating SyncDogData");
		return 1;
	}
	return 0;
}

void storeDir(library *lb) {
	struct stat info;
	struct dirent *entry;
	char filepath[512];
	printf("Enter filepath\n");
	fgets(filepath, 512, stdin);
	filepath[strcspn(filepath, "\n")] = '\0';
	char *dirslash = strrchr(filepath, '/');
	if(dirslash == NULL) {
		printf("%s is NULL\n", filepath);
		return;
	}
	if(strcmp(dirslash, "/")) {
		strcat(filepath, "/");
	}
	printf("searching in: %s\n", filepath);
	sleep(2);
	DIR *dir = opendir(filepath);
	if(dir == NULL) {
		perror("ERR opening directory");
		return;
	}
	while((entry = readdir(dir)) != NULL) {
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			printf("skipped special registries\n");
			continue;
		}else{
			char *fileType = strrchr(entry->d_name, '.');
			if(fileType == NULL) {
				printf("%s is NULL or not file?\n", entry->d_name);
				continue;
			}
			if(strcmp(fileType, ".mp3") == 0) {
				char musicpath[512];
				int result = snprintf(musicpath, sizeof(musicpath), "%s%s", filepath, entry->d_name);
				if(result < 0 || (size_t)result >= sizeof(musicpath)) {
					printf("path too long");
					closedir(dir);
					return;
				}
				if(stat(musicpath, &info) == -1) {
					printf("Can't read: %s\n", entry->d_name);
					sleep(1);
					continue;
				}
				if(S_ISREG(info.st_mode)) {
					if(lb->count == lb->capacity) {
						printf("realloc needed\n");
						sleep(1);
						mp3 *temp = realloc(lb->stored, sizeof(*lb->stored) * (lb->capacity * 2));
						if(temp == NULL) {
							printf("ERR memory reallocation failed");
							closedir(dir);
							return;
						}
						lb->capacity = lb->capacity * 2;
						printf("success, new space: %zu bytes\n", sizeof(*lb->stored) * lb->capacity);
						lb->stored = temp;
					}
					strcpy(lb->stored[lb->count].path, musicpath);
					printf("%s %zu\n", lb->stored[lb->count].path, lb->count);

					strcpy(lb->stored[lb->count].name, entry->d_name);

					lb->stored[lb->count].size = info.st_size;
					printf("Size: %zu bytes\n", lb->stored[lb->count].size);
					lb->count++;
				}
			}
		}
	}

	int temp;
	printf("\n\nType anything to continue\n");
	scanf("%i", &temp);
	clearbuffer();

	if(stat("SyncDogData", &info) == -1) {
		if(createData() == 1) {
			closedir(dir);
			return;
		}
		else {
			if(stat("SyncDogData", &info) == -1) {
				closedir(dir);
				return;
			}
		}
	}
	if(S_ISDIR(info.st_mode) == 1) {
		FILE *file = fopen("SyncDogData/SyncDogData.txt", "w");
		if(file == NULL) {
			perror("ERR while opening file");
			closedir(dir);
			return;
		}
		fprintf(file, "%s", filepath);
		fclose(file);
	}
	else {
		printf("ERR SyncDogData is not a directory, manual intervention required\n");
		closedir(dir);
		return;
	}
	closedir(dir);
}
void structTest(library *lb) {
	printf("RUNNING STRUCT TEST\n");
	for(size_t i = 0;i < lb->count;i++) {
		printf("%s\n%s\nSize: %zu bytes\n", lb->stored[i].path, lb->stored[i].name, lb->stored[i].size);
	}
}

void startSync() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("ERR socket failed");
		return;
	}
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(50600);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	int opt = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("ERR setsockopt failed");
		close(sock);
		return;
	}
	int bd = bind(sock, (struct sockaddr *)&server, sizeof(server));
	if(bd == -1) {
		perror("ERR bind failed");
		close(sock);
		return;
	}
	int lis = listen(sock, 1);
	if(lis == -1) {
		perror("ERR listen failed");
		close(sock);
		return;
	}
	struct sockaddr client;
	socklen_t client_len = sizeof(client);
	printf("Server running, awaiting client...\n");
	int client_fd = accept(sock, (struct sockaddr *)&client, &client_len);
	if(client_fd == -1) {
		perror("ERR accept failed");
		close(sock);
		return;
	}
	char msg[14];
	int rv = recv(client_fd, msg, sizeof(msg) -1 , 0);
	if(rv == -1) {
		perror("ERR recv failed");
		close(sock);
		close(client_fd);
		return;
	}
	msg[rv] = '\0';
	if(strcmp(msg, "HELLOSYNCDOG") == 0) {
		printf("Connection successful...\n");
	}else{
		printf("Connection refused\n");
	}
	close(sock);
	close(client_fd);
	return;
}

int main() {
	library lb = {0};
	lb.capacity = 100;
	mp3 *temp = malloc(sizeof(*lb.stored) * lb.capacity);
	if(temp == NULL) {
		printf("ERR memory allocation failed");
		return 1;
	}
	lb.stored = temp;
	int option = 0;
	do {
		printf("SyncDog\n\n1-Store library\n2-Start sync\n3-Exit\n");
		if(scanf("%i", &option) != 1) {
			printf("numbers only\n");
			sleep(1);
			system("clear");
			continue;
		}
		clearbuffer();
		switch(option) {
			case 1:
				system("clear");
				storeDir(&lb);
				break;
			case 2:
				system("clear");
				startSync();
				break;
			case 3:
				printf("Goodbye\n");
				break;
			default:
				printf("not a option\n");
				sleep(1);
				system("clear");
				break;
		}
	}while(option != 3);
	free(lb.stored);
	return 0;
}
