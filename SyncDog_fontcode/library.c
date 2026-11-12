#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
FILE *file;
char *SyncDogData_directory = "SyncDogData";
char dirLocation[128];
typedef struct {
	char name[256];
	char path[512];
	long size;
}mp3;
typedef struct {
	mp3 *stored;
	size_t count;
	size_t capacity;
}musicLibrary;
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
		sleep(2);
		return 1;
	}
	printf("Opened %s with success\n", dirLocation);
	sleep(1);
	struct stat directoryInfo;
	if(stat(SyncDogData_directory, &directoryInfo) == 0) {
		if(S_ISDIR(directoryInfo.st_mode)) {
			printf("SyncDogData exists and is a directory\n");
			sleep(1);
		}else{
			printf("SyncDogData non existent... creating new directory\n");
			sleep(1);
			if(mkdir(SyncDogData_directory, 0700) == 0) {
				printf("done\n");
				sleep(1);
			}else{
				perror("ERR failed to create new directory\n");
				sleep(1);
				return 1;
			}
		}

	}else{
		printf("SyncDogData non existent... creating new directory\n");
		sleep(1);
		if(mkdir(SyncDogData_directory, 0700) == 0) {
			printf("done\n");
			sleep(1);
		}else{
			perror("ERR failed to create new directory\n");
			sleep(1);
			return 1;
		}
	}
	file = fopen("SyncDogData/savedDir.txt", "w");
	if(file == NULL) {
		perror("ERR failed to open savedDir.txt\n");
		sleep(2);
		return 1;
	}
	fprintf(file, "%s", dirLocation);
	printf("success\n");
	sleep(1);
	fclose(file);
	closedir(dir);
	return 0;
}
int openDir(musicLibrary *library) {
	struct stat chart;
	file = fopen("SyncDogData/savedDir.txt", "r");
	if(file == NULL) {
		perror("ERR failed to open file! or file not found?\n");
		sleep(1);
		return 0;
	}
	if(fgets(dirLocation, sizeof(dirLocation), file) == NULL) {
		perror("ERR while reading dirLocation\n");
		fclose(file);
		return 0;
	}
	dirLocation[strcspn(dirLocation, "\n")] = '\0';
	DIR *dir = opendir(dirLocation);
	if(dir == NULL) {
		perror("ERR failed to open directory! try define main directory\n");
		fclose(file);
		sleep(2);
		return 0;
	}
	printf("finding only mp3...\n\n");
	sleep(1);
	mp3 *tempAlbum;
	int ignore = 0;
	struct dirent *data;
	char filepath[512];
	while((data = readdir(dir)) != NULL) {
		char *pos = strrchr(data->d_name, '.');
		if(pos == NULL) {
			ignore++;
			continue;
		}
		if(strcmp(data->d_name, ".") != 0 && strcmp(data->d_name, "..") != 0) {
			if(!strcmp(pos, ".mp3")) {
				int result = snprintf(filepath, sizeof(filepath), "%s/%s", dirLocation, data->d_name);
				if(result >= 0 && (size_t)result < sizeof(filepath)) {
					if(stat(filepath, &chart) == 0) {
						if(!S_ISREG(chart.st_mode)) {
							ignore++;
						}else{
							if(library->count == library->capacity) {
								library->capacity *= 2;
								tempAlbum = realloc(library->stored, library->capacity * sizeof(mp3));
								if(tempAlbum == NULL) {
									perror("ERR out of memory\n");
									sleep(1);
									closedir(dir);
									fclose(file);
									return 0;
								}else{
								library->stored = tempAlbum
								}
							}
							printf("%s\n", data->d_name);
							strcpy(library->stored[library->count].name, data->d_name);
							strcpy(library->stored[library->count].path, filepath);
							library->stored[library->count].size = chart.st_size;
							library->count++;
						}
					}else{
						perror("ERR stat failed\n");
						sleep(1);
						closedir(dir);
						fclose(file);
						return 0;
				}
			}else{
				ignore++;
			}
		}
	}
	closedir(dir);
	fclose(file);
	printf("\nall data read successfully\n%i files found\n%i files are not .mp3\n\nPress enter to continue\n", library->count, ignore);
	getchar();
	return 0;
}
int main() {
	musicLibrary library = {0};
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
					check = openDir(&library);
				}while(check);
				break;
			case '3':
				break;
			default:
				printf("ERR not a option\n");
				sleep(1);
		}
	}while(op != '3');
	free(library.stored);
	return 0;
}
