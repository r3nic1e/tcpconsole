//
// Created by mlosev on 04.04.17.
//

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/klog.h>
#include <assert.h>
#include <stdint.h>
#include <sched.h>
#include <limits.h>

#ifndef TCPCONSOLE_DEFINES_H
#define TCPCONSOLE_DEFINES_H

#define DEFAULT_LISTEN_PORT 4095
#define DEFAULT_GATEWAY_IP "192.168.0.1"

#ifndef VERSION
#define VERSION "dev"
#endif

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15
#endif

typedef struct {
	int sysrq_fd;
	int vcsa0_fd;        /* virtual console 0 */
	int drop_caches_fd;
	int kmsg_fd;
	char hostname[HOST_NAME_MAX+1];

	char *dmesg_buffer;
	size_t dmesg_buffer_size;

} parameters_t;

#endif //TCPCONSOLE_DEFINES_H
