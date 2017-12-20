//
// Created by mlosev on 31.03.17.
//

#include "defines.h"
#include "error.h"

int WRITE(int sock, char *s, size_t len) {
	while (len > 0) {
		ssize_t rc = write(sock, s, len);

		if (rc == -1) {
			if (errno == EINTR)
				continue;

			return -1;
		}
		if (rc == 0)
			return -1;

		len -= rc;
		s += rc;
	}

	return 0;
}

int readchar(int fd) {
	for (;;) {
		char key;
		ssize_t rc = read(fd, &key, 1);

		if (rc == -1) {
			if (errno == EINTR)
				continue;

			break;
		} else if (rc == 0) {
			break;
		}

		return (int) key;
	}

	return -1;
}

int flush_socket(int fd) {
	for (;;) {
		static char buffer[16];
		long count = 0;
		int rc;
		struct timeval tv;
		fd_set rfds;

		tv.tv_sec = 0;
		tv.tv_usec = 100;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		rc = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (rc == -1) {
			if (errno == EINTR)
				continue;

			return -1;
		}
		if (rc == 0) {
			break;
		}

		if (FD_ISSET(fd, &rfds)) {
			count = read(fd, buffer, sizeof(buffer));
		}

		//connection is closed
		if (count == 0) {
			return -1;
		}
	}

	return 0;
}

char *get_string(int fd) {
	static char buffer[128];
	size_t len = 0;

	if (flush_socket(fd) == -1)
		return NULL;

	do {
		int key = readchar(fd);
		if (key == -1) {
			return NULL;
		}

		if (key == 10 || key == 13) {
			break;
		}

		buffer[len++] = (char) key;
	} while (len < (sizeof(buffer) - 1));
	buffer[len] = 0x00;

	return strdup(buffer);
}

int kmsgprint(int fd, char *format, ...) {
	size_t len;
	static char buffer[4096];
	va_list ap;

	va_start(ap, format);
	len = (size_t) vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	static char buffer2[5000];
	size_t len2;

	char *prefix = "tcpconsole: ";
	len2 = len + strlen(prefix);

	memset(buffer2, 0, sizeof(buffer2));
	strcpy(buffer2, prefix);
	strcat(buffer2, buffer);

	return WRITE(fd, buffer2, len2);
}

int sockprint(int fd, char *format, ...) {
	size_t len;
	static char buffer[4096];
	va_list ap;

	va_start(ap, format);
	len = (size_t) vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	return WRITE(fd, buffer, len);
}

int sockerror(int fd, char *what) {
	return sockprint(fd, "error on %s: %s (%d)\n", what, strerror(errno), errno);
}

int verify_password(int client_fd, char *password) {
	char *entered;
	char suppress_goahead[] = {(char)0xff, (char)0xfb, 0x03};
	char dont_linemode[] = {(char)0xff, (char)0xfe, 0x22};
	char dont_new_env[] = {(char)0xff, (char)0xfe, 0x27};
	char will_echo[] = {(char)0xff, (char)0xfb, 0x01};
	char dont_echo[] = {(char)0xff, (char)0xfe, 0x01};

	WRITE(client_fd, suppress_goahead, sizeof suppress_goahead);
	WRITE(client_fd, dont_linemode, sizeof dont_linemode);
	WRITE(client_fd, dont_new_env, sizeof dont_new_env);
	WRITE(client_fd, will_echo, sizeof will_echo);
	WRITE(client_fd, dont_echo, sizeof dont_echo);

	if (sockprint(client_fd, "Password: ") == -1) {
		return -1;
	}

	entered = get_string(client_fd);
	if (!entered) {
		return -1;
	}

	if (strcmp(password, entered) == 0) {
		free(entered);
		return 0;
	}

	free(entered);

	return -1;
}

void write_pidfile(char *file) {
	FILE *pidf = fopen(file, "w");
	if (pidf == NULL) {
		error_exit("Error creating pid-file %s\n", file);
	}

	fprintf(pidf, "%d\n", getpid());

	fclose(pidf);
}

int open_file(char *path, int mode) {
	int fd = open(path, mode);
	if (fd == -1) {
		error_exit("open_file(%s) failed", path);
	}

	return fd;
}

char *read_password(char *file) {
	char buffer[128], *pw, *lf;
	struct stat buf;
	int fd = open_file(file, O_RDONLY);
	ssize_t rc;

	if (fstat(fd, &buf) == -1) {
		error_exit("fstat(%s) failed", file);
	}

	rc = read(fd, buffer, sizeof(buffer) - 1);
	if (rc == -1) {
		error_exit("error reading password");
	}
	buffer[rc] = 0x00;

	lf = strchr(buffer, '\n');
	if (lf) {
		*lf = 0x00;
	}

	close(fd);

	pw = strdup(buffer);
	if (!pw) {
		error_exit("strdup() failed");
	}

	return pw;
}