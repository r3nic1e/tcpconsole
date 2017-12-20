/* program must be invoked from /etc/inittab
 */
#include "defines.h"
#include "error.h"
#include "io.h"
#include "serve.h"

int main(int argc, char *argv[]) {
	char *password, *env_port;
	uint16_t port = DEFAULT_LISTEN_PORT;
	parameters_t pars;
	struct sched_param sched_par;

	env_port = getenv("TCPCONSOLE_LISTEN_PORT");

	if (env_port) {
		port = (uint16_t) atoi(env_port);
	}

	openlog("tcpconsole", LOG_CONS | LOG_NDELAY | LOG_NOWAIT | LOG_PID, LOG_DAEMON);

	if (getuid()) {
		error_exit("This program must be invoked with root-rights.");
	}

	password = read_password("/etc/tcpconsole.pw");

	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		error_exit("signal(SIGTERM) failed");
	}

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		error_exit("signal(SIGHUP) failed");
	}

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		error_exit("signal(SIGPIPE) failed");
	}

	pars.sysrq_fd = open_file("/proc/sysrq-trigger", O_WRONLY);
	pars.vcsa0_fd = open_file("/dev/vcsa", O_RDONLY);
	pars.drop_caches_fd = open_file("/proc/sys/vm/drop_caches", O_WRONLY);
	pars.kmsg_fd = open_file("/dev/kmsg", O_WRONLY);


	pars.hostname[HOST_NAME_MAX] = '\0';
	
        if (gethostname(pars.hostname, HOST_NAME_MAX) == -1) {
            //set empty hostname on failure
            sprintf(pars.hostname, "<>");
        }

	if (setpriority(PRIO_PROCESS, 0, -10) == -1) {
		error_exit("Setpriority failed");
	}

	if (nice(-20) == -1) {
		error_exit("Failed to set nice-value to -20");
	}

	if (mlockall(MCL_CURRENT) == -1 || mlockall(MCL_FUTURE) == -1) {
		error_exit("Failed to lock program in core");
	}

	memset(&sched_par, 0x00, sizeof(sched_par));
	sched_par.sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &sched_par) == -1) {
		error_exit
				("Failed to set scheduler properties for this process");
	}

	syslog(LOG_INFO, "tcpconsole started");

	write_pidfile("/var/run/tcpconsole.pid");

	if ((pars.dmesg_buffer_size = (size_t) klogctl(10, NULL, 0)) == -1) {
		error_exit("klogctl(10) failed");
	}
	pars.dmesg_buffer = (char *) malloc(pars.dmesg_buffer_size + 1);
	if (!pars.dmesg_buffer) {
		error_exit("malloc failure");
	}

	listen_on_socket(port, &pars, password);

	return 1;
}

