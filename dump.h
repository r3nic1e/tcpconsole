//
// Created by mlosev on 31.03.17.
//

#ifndef TCPCONSOLE_DUMP_H
#define TCPCONSOLE_DUMP_H

int dump_virtual_console(int fd_out, int fd_in);

int dump_dmesg(int fd, char *dmesg_buffer, int dmesg_buffer_size, char clear);

int set_dmesg_loglevel(int fd, int level);

int dump_loadavg(int fd);

int dump_ps(int fd);

#endif //TCPCONSOLE_DUMP_H
