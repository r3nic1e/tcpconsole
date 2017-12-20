//
// Created by mlosev on 31.03.17.
//

#ifndef TCPCONSOLE_KILL_H
#define TCPCONSOLE_KILL_H

int kill_one_proc(int, int kmsg_fd);

int kill_procs(int, int kmsg_fd);

#endif //TCPCONSOLE_KILL_H
