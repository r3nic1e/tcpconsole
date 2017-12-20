//
// Created by mlosev on 04.04.17.
//

#include "defines.h"
#include "io.h"

int ec_help(int fd) {
	int rc = 0;

	rc |=
			sockprint(fd,
					  "tcpconsole v " VERSION
							  ", (C) 2009-2012 by folkert@vanheusden.com\n");

	// userspace hotkeys
	rc |= sockprint(fd, "h: this help\n");
	rc |= sockprint(fd, "d: dump virtual console 0\n");
	rc |= sockprint(fd, "j: 'kill -NUM' for a given pid\n");
	rc |= sockprint(fd, "k: 'killall -NUM' for a given name\n");
	rc |= sockprint(fd, "l: dump dmesg\n");
	rc |= sockprint(fd, "a: drop cache\n");
	rc |= sockprint(fd, "m: dump dmesg & clear dmesg buffer\n");
	rc |= sockprint(fd, "p: process list\n");
	rc |= sockprint(fd, "i: show system (e.g. load)\n");
	rc |= sockprint(fd, "1-8: set dmesg loglevel\n");
	rc |= sockprint(fd, "q: log off\n");

	// SysRq hotkeys
	rc |= sockprint(fd, "\nSysreq:\n");
	rc |= sockprint(fd, "B - boot\n");
	rc |= sockprint(fd, "C - kexec\n");
	rc |= sockprint(fd, "D - list all locks\n");
	rc |=
			sockprint(fd,
					  "E - SIGTERM to all but init, I - SIGKILL to all but init\n");
	rc |= sockprint(fd, "F - call oom_kill\n");
	rc |= sockprint(fd, "L - backtrace\n");
	rc |= sockprint(fd, "M - memory info dump, P - register/flags dump\n");
	rc |= sockprint(fd, "O - switch off\n");
	rc |= sockprint(fd, "Q - list hrtimers\n");
	rc |= sockprint(fd, "S - SYNC, U - umount\n");
	rc |= sockprint(fd, "T - tasklist dump, W - unint. tasks dump\n");

	return rc;
}