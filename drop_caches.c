//
// Created by mlosev on 04.04.17.
//

#include "io.h"

int drop_caches(int fd, int drop_caches_fd, int kmsg_fd) {
	char *key;
	sockprint(fd, "Enter type of cache:\n1. drop pagecache\n2. drop slabs (inodes, etc.)\n3. drop both\n > ");
	key = get_string(fd);

	if (!key) {
		return -1;
	}

	switch (*key) {
		case '1':
		case '2':
		case '3':
			break;
		default:
			if (sockprint(fd, "\nError: need code range from 1 to 3\n") == -1) {
				return -1;
			}
			return 0;
	}

	kmsgprint(kmsg_fd, "Drop caches with type %s\n", key);

	if (WRITE(drop_caches_fd, key, 1) == -1) {
		if (sockprint(fd, "error writing %c", key) == -1) {
			return -1;
		}
	}

	sockprint(fd, "\nThe value successfully saved.\n");
	return 0;
}