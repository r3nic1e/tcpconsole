//
// Created by mlosev on 31.03.17.
//

#ifndef TCPCONSOLE_IO_H
#define TCPCONSOLE_IO_H

int WRITE(int sock, char *s, unsigned long len);

int readchar(int fd);

int flush_socket(int fd);

char *get_string(int fd);

int sockerror(int fd, char *what);

int sockprint(int fd, char *format, ...);

int kmsgprint(int fd, char *format, ...);

int verify_password(int client_fd, char *password);

void write_pidfile(char *file);

int open_file(char *path, int mode);

char *read_password(char *file);

#endif //TCPCONSOLE_IO_H
