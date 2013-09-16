#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define KEY "BADSECTORbadsectorBaDsEcToRbAdSeCtOrBADSECTOR"
#define KEYMATCH 9

int max_keys = 0;
char *keys;

int expand_key() {
	printf("Searching for '%s'\n", KEY);

	// find the max_keys
	int i;
	for(i = 0; i < strlen(KEY) ;i++) {
		int count = 0;
		int j;

		for(j = 0; j < strlen(KEY) ;j++)
			if(KEY[j] == KEY[i]) count++;

		if(count > max_keys) max_keys = count;
	}

	printf("Max keys per character: %d\n", max_keys);
	printf("Min characters to match: %d\n", KEYMATCH);

	// build the key matches
	int mem = 256 * max_keys * KEYMATCH;
	printf("Allocating %d mem for keys\n", mem);

	keys = malloc(mem);
	if(!keys) {
		perror("Cannot allocate memory");
		exit(1);
	}

	char k[strlen(KEY)*2+1];
	strcpy(k, KEY);
	strcat(k, KEY);

	for(i = 0; i < ((strlen(KEY)*2)-KEYMATCH) ; i++) {
		char f[KEYMATCH + 1];
		int j;
		int stored=0;

		strncpy(&f[0], &k[i], KEYMATCH);

		for(j = 0 ; j < max_keys ;j++) {
			int byte = ((unsigned char)f[0] * max_keys * KEYMATCH) + (j * KEYMATCH);
			char *s = &keys[byte];

			if(strncmp(s, &f[0], KEYMATCH) == 0) {
				//printf("Ignoring key '%s'. Already exists at position %d (byte %d).\n", &f[0], j, byte);
				stored = 1;
				break;
			}
			else if(*s)
				continue;
			else {
				strncpy(s, &f[0], KEYMATCH);
				//printf("Copying key '%s' at position %d (byte %d).\n", &f[0], j, byte);
				stored = 1;
				break;
			}
		}
		if(!stored) {
			printf("Cannot store key '%s' anywhere.\n", &f[0]);
			exit(1);
		}
	}

	return 0;
}

int search(char *map, off_t length) {
	off_t i;
	int j;
	int found = 0;
	off_t ll = length - KEYMATCH;

	char *rkeys = keys;
	int rmax_keys = max_keys;

	for(i = 0; i < ll ;i++) {
		for(j = 0; j < rmax_keys ;j++) {
			char *s = &rkeys[((unsigned char)map[i] * rmax_keys * KEYMATCH) + (j * KEYMATCH)];

			if(*s) {
				int x;
				int thisisit = 1;
				for(x = 0; x < KEYMATCH ;x++) {
					if(map[i+x] != s[x]) {
						thisisit = 0;
						break;
					}
				}
				if(thisisit) {
					char k[KEYMATCH+1];
					strncpy(k, &rkeys[((unsigned char)map[i] * rmax_keys * KEYMATCH) + (j * KEYMATCH)], KEYMATCH);
					k[KEYMATCH] = '\0';
					printf(" matches '%s' at position %lu.\n", k, i);
					found = 1;
					break;
				}
			}
		}
		if(found) break;
	}

	if(!found) printf(" OK\n");

	return 0;
}

int main(int argc, char **argv) {
	int i;
	int fd;
	char *map;
	struct stat sb;
	expand_key();

	for(i = 1; i < argc ;i++) {
		// printf("File '%s'\n", argv[i]);
		fd = open(argv[i], O_RDONLY|O_NOFOLLOW|O_LARGEFILE);
		if(fd == -1) {
			perror("Cannot open file.");
			continue;
		}

		if(fstat(fd, &sb) == -1) {
			perror("Cannot get file size.");
			close(fd);
			continue;
		}

		printf("%s: (%lu MB) ", argv[i], sb.st_size / 1024 / 1024);
		fflush(stdout);
		map = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if(map == MAP_FAILED) {
			perror("Cannot map file.");
			close(fd);
			continue;
		}

		search(map, sb.st_size);

		munmap(map, sb.st_size);
		close(fd);
	}
	return 0;
}
