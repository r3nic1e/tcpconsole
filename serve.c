//
// Created by mlosev on 31.03.17.
//

#include "defines.h"
#include "io.h"
#include "dump.h"
#include "kill.h"
#include "error.h"
#include "help.h"
#include "sysrq.h"
#include "drop_caches.h"

void serve_client(int fd, parameters_t *pars) {
	sockprint(fd, "Enter 'h' for help\n");

	for (;;) {
		int key;

		if (sockprint(fd, "(%s) tcpconsole > ", pars->hostname) == -1) {
			break;
		}

		if ((key = readchar(fd)) == -1) {
			break;
		}

		if (key < 32 || key > 126) {
			if (sockprint(fd, "\r") == -1)
				break;
			continue;
		}

		if (sockprint(fd, "%c\n", key) == -1) {
			break;
		}

		switch (key) {
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				if (set_dmesg_loglevel(fd, key - '0') == -1) {
					return;
				}
				break;

			case 'd':
				if (dump_virtual_console(fd, pars->vcsa0_fd) == -1) {
					return;
				}
				break;

			case '?':
			case 'h':
				if (ec_help(fd) == -1) {
					return;
				}
				break;

			case 'i':
				if (dump_loadavg(fd) == -1) {
					return;
				}
				break;

			case 'j':
				if (kill_one_proc(fd, pars->kmsg_fd) == -1) {
					return;
				}
				break;

			case 'k':
				if (kill_procs(fd, pars->kmsg_fd) == -1) {
					return;
				}
				break;

			case 'l':
				if (dump_dmesg(fd, pars->dmesg_buffer, (int) pars->dmesg_buffer_size, 0) == -1) {
					return;
				}
				break;

			case 'm':
				if (dump_dmesg(fd, pars->dmesg_buffer, (int) pars->dmesg_buffer_size, 1) == -1) {
					return;
				}
				break;

			case 'p':
				if (dump_ps(fd) == -1) {
					return;
				}
				break;

			case 'q':
				return;
			case 'a':
				if ((drop_caches(fd, pars->drop_caches_fd, pars->kmsg_fd) == -1)) {
					return;
				}
				break;
			case 10:
			case 13:
				break;
			default:
				if (isupper(key)) {
					do_sysreq(fd, (char) tolower(key), pars->sysrq_fd, pars->kmsg_fd);
				} else {
					sockprint(fd, "'%c' is not understood\n", key);
				}
				break;
		}
	}
}

/* http://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine */
in_addr_t get_local_addr() {
	struct sockaddr_in serv, name;
	int sock, err;

	socklen_t namelen = sizeof(name);

	const char *gateway_ip = DEFAULT_GATEWAY_IP;
	uint16_t test_port = 53;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock != -1);

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(gateway_ip);
	serv.sin_port = htons(test_port);

	err = connect(sock, (const struct sockaddr *) &serv, sizeof(serv));
	assert(err != -1);

	err = getsockname(sock, (struct sockaddr *) &name, &namelen);
	assert(err != -1);

	close(sock);

	return name.sin_addr.s_addr;
}

void listen_on_socket(uint16_t port, parameters_t *pars, char *password) {
	int server_fd;
	struct sockaddr_in server_addr;
	int on = 1, sec60 = 60;
	socklen_t optlen;

	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = get_local_addr();

	syslog(LOG_INFO, "Starting listen on %s:%d", inet_ntoa(server_addr.sin_addr), port);

	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		error_exit("error creating socket");
	}

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
               syslog(LOG_INFO, "setsockopt(SO_REUSEADDR) failed");
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == -1) {
                syslog(LOG_INFO, "setsockopt(SO_REUSEPORT) failed");
        }

	if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		error_exit("bind() failed");
	}

	if (listen(server_fd, SOMAXCONN)) {
		error_exit("listen(%d) failed", SOMAXCONN);
	}

	if (setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPIDLE, &sec60, sizeof(sec60)) == -1) {
		error_exit("setsockopt(TCP_KEEPIDLE) failed");
	}

	if (setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPINTVL, &sec60, sizeof(sec60)) == -1) {
		error_exit("setsockopt(TCP_KEEPINTVL) failed");
	}

	syslog(LOG_PID | LOG_DAEMON, "Listening on %s:%d", inet_ntoa(server_addr.sin_addr), port);

	kmsgprint(pars->kmsg_fd, "Listening on %s:%d\n", inet_ntoa(server_addr.sin_addr), port);

	for (;;) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_size = sizeof(client_addr);
		int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_size);
		if (client_fd == -1) {
			if (errno == EINTR)
				continue;

			sleep(1);
			continue;
		}

		optlen = sizeof(on);
		if (setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &on, optlen) == -1) {
			if (sockerror(client_fd, "setsockopt(SO_KEEPALIVE)") == -1) {
				close(client_fd);
				continue;
			}
		}

		if (verify_password(client_fd, password) == 0) {
			kmsgprint(pars->kmsg_fd, "Accepting client from %s:%d\n",
					  inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
			serve_client(client_fd, pars);
		}

		close(client_fd);
	}
}
