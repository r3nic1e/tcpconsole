//
// Created by mlosev on 04.04.17.
//

#include "io.h"

int do_sysreq(int fd, char key, int sysreq_fd, int kmsg_fd) {
	int yn;

	if (key < 'a' || key > 'z') {
		return sockprint(fd, "key out of range\n");
	}

	if (sockprint(fd, "Send %c to sysreq? (y/n)\n", key) == -1) {
		return -1;
	}

	do {
		yn = readchar(fd);
	} while (yn != 'y' && yn != 'n' && yn != -1);

	if (yn == 'y') {
		kmsgprint(kmsg_fd, "Sending '%c' to sysrq-trigger\n", key);
		if (WRITE(sysreq_fd, &key, 1) == -1)
			return sockerror(fd, "WRITE(sysreq_fd)");
	}

	return yn == -1 ? -1 : 0;
}