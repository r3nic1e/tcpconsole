//
// Created by mlosev on 31.03.17.
//

#ifndef TCPCONSOLE_SERVE_H
#define TCPCONSOLE_SERVE_H

void serve_client(int, parameters_t *);
void listen_on_socket(uint16_t port, parameters_t *pars, char *password);
in_addr_t get_local_addr();

#endif //TCPCONSOLE_SERVE_H
