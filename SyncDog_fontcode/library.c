#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
FILE *file;
char dirLocation[99];
void clearBuffer() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}
int defineDir() {
	printf("Localization of the main directory? type leave to leave\n");
	fgets(dirLocation, sizeof(dirLocation), stdin);
	dirLocation[strcspn(dirLocation, "\n")] = '\0';
	if(strcmp(dirLocation, "leave") == 0) {
		return 0;
	}
	DIR *dir = opendir(dirLocation);
	if(dir == NULL) {
		perror("ERR wrong directory maybe?");
		sleep(3);
		return 1;
	}
	printf("Opened %s with success\n", dirLocation);
	sleep(2);
	file = fopen("SyncDogData/savedDir.txt", "w");
	if(file == NULL) {
		perror("ERR failed to open file!");
		sleep(3);
		return 1;
	}
	fprintf(file, "%s", dirLocation);
	fclose(file);
	return 0;
}
int openDir() {
	file = fopen("SyncDogData/savedDir.txt", "r");
	if(file == NULL) {
		perror("ERR failed to open file! or file not found?");
		sleep(3);
		return 0;
	}
	fgets(dirLocation, sizeof(dirLocation), file);
	dirLocation[strcspn(dirLocation, "\n")] = '\0';
	DIR *dir = opendir(dirLocation);
	if(dir == NULL) {
		perror("ERR failed to open directory! try define main directory");
		sleep(3);
		return 0;
	}
	int max = 0;
	struct dirent *data;
	while((data = readdir(dir)) != NULL) {
		if(strcmp(data->d_name, ".") != 0 && strcmp(data->d_name, "..") != 0) {
			printf("%s\n", data->d_name);
			max++;
		}
	}
	closedir(dir);
	printf("\nall data read successfully\n%i files found\n\nPress enter to continue\n", max);
	getchar();
	return 0;
}
int main() {
	char op = '0';
	int check = 1;
	do {
		system("clear");
		printf("1-define main directory\n2-execute\n3-exit\n");
		scanf(" %c", &op);
		clearBuffer();
		switch(op) {
			case '1':
				do {
					check = defineDir();
				}while(check);
				break;
			case '2':
				do {
					check = openDir();
				}while(check);
				break;
			case '3':
				break;
			default:
				printf("ERR not a option\n");
				sleep(1);
		}
	}while(op != '3');
	return 0;
}
