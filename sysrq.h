//
// Created by mlosev on 04.04.17.
//

#ifndef TCPCONSOLE_SYSRQ_H
#define TCPCONSOLE_SYSRQ_H

int do_sysreq(int fd, char key, int sysreq_fd, int kmsg_fd);

#endif //TCPCONSOLE_SYSRQ_H
