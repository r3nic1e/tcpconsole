//
// Created by mlosev on 31.03.17.
//

#include "defines.h"
#include "io.h"

int kill_one_proc(int client_fd, int kmsg_fd) {
	int rc = 0, signum = 0;
	__pid_t pid;
	char *entered;

	// get process pid
	if (sockprint(client_fd, "Process id (PID, q to abort): ") == -1) {
		return -1;
	}

	entered = get_string(client_fd);
	if (!entered) {
		return -1;
	}

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	pid = atoi(entered);

	if (abs(pid) == 1) {
		sockprint(client_fd, "Attempting to kill init, aborting\n", pid);
		free(entered);
		return -1;
	}

	// get signal number
	if (sockprint(client_fd, "Signal number (0-25, q to abort) [0]: ") == -1)
		return -1;

	entered = get_string(client_fd);
	if (!entered)
		return -1;

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	signum = atoi(entered);

	if (signum < 0 || signum > 25) {
		sockprint(client_fd, "Invalid signal (%d) specified (must be in range 0-25), aborting\n", signum);
		free(entered);
		return 0;
	}

	kmsgprint(kmsg_fd, "Killing pid %d with signal %d\n", pid, signum);

	rc = sockprint(client_fd, "Killing pid %d with signal %d\n", pid, signum);

	if (kill(pid, signum) == -1) {
		rc |= sockerror(client_fd, "kill failed");
	}

	return rc;
}

int kill_procs(int client_fd, int kmsg_fd) {
	int nprocs = 0, use_cmdline = 0, signum = 0, rc = 0;
	struct dirent *de;
	DIR *dirp = opendir("/proc");
	char *entered;
	static char process_name[120];

	// get process name
	if (sockprint(client_fd, "Process name (q to abort): ") == -1)
		return -1;

	entered = get_string(client_fd);
	if (!entered)
		return -1;

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	strcpy(process_name, entered);

	// get signal number
	if (sockprint(client_fd, "Signal number (0-25, q to abort) [0]: ") == -1)
		return -1;

	entered = get_string(client_fd);
	if (!entered)
		return -1;

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	signum = atoi(entered);

	if (signum < 0 || signum > 25) {
		sockprint(client_fd, "Invalid signal (%d) specified (must be in range 0-25), aborting\n", signum);
		free(entered);
		return 0;
	}

	// get use_cmdline option
	if (sockprint(client_fd, "Use cmdline? (0-1, q to abort) [0]: ") == -1)
		return -1;

	entered = get_string(client_fd);
	if (!entered)
		return -1;

	if (strcmp(entered, "q") == 0) {
		free(entered);
		return 0;
	}

	use_cmdline = atoi(entered);

	kmsgprint(kmsg_fd, "Killing process '%s' with signal %d (use_cmdline=%d)\n", process_name, signum, use_cmdline);

	while ((de = readdir(dirp)) != NULL) {
		if (!isdigit(de->d_name[0]))
			continue;

		FILE *fh;
		static char path[128];

		int killed = 0;

		snprintf(path, sizeof(path), "/proc/%s/stat", de->d_name);
		fh = fopen(path, "r");
		if (fh) {
			static char fname[4096], dummystr[2];
			int dummy;
			rc = fscanf(fh,
						"%d %s %c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
						&dummy, fname, &dummystr[0], &dummy,
						&dummy, &dummy, &dummy, &dummy, &dummy,
						&dummy, &dummy, &dummy, &dummy, &dummy,
						&dummy, &dummy, &dummy, &dummy, &dummy,
						&dummy, &dummy, &dummy, &dummy, &dummy);

			if (rc == -1) {
				fclose(fh);
				sockerror(client_fd, "fscanf error");
				continue;
			}

			if (strstr(fname, process_name) != NULL) {
				pid_t pid = atoi(de->d_name);
				sockprint(client_fd, "Killing pid %d (%s) with signal %d: %s\n", pid, dummystr, signum, fname);

				if (kill(pid, signum) == -1) {
					sockerror(client_fd, "kill failed");
				}

				nprocs++;

				killed = 1;
			}

			fclose(fh);
		}

		if (killed == 1)
			continue;

		if (!use_cmdline)
			continue;

		//cmdline code here
		snprintf(path, sizeof(path), "/proc/%s/cmdline", de->d_name);
		fh = fopen(path, "r");

		if (fh) {
			size_t len;
			unsigned int loop;
			static char cmdline[4096];

			len = fread(cmdline, 1, sizeof(cmdline) - 1, fh);
			if (len > 0) {
				cmdline[len] = 0x00;
				for (loop = 0; loop < len; loop++) {
					if (cmdline[loop] == 0x00)
						cmdline[loop] = ' ';
				}

				if (strstr(cmdline, process_name) != NULL) {
					pid_t pid = atoi(de->d_name);
					sockprint(client_fd, "Killing pid %d with signal %d: %s\n", pid, signum, cmdline);

					if (kill(pid, signum) == -1) {
						sockerror(client_fd, "kill failed");
					}

					nprocs++;
				}
			}

			fclose(fh);
		}
		//cmdline code ends here
	}

	closedir(dirp);
	free(entered);

	return sockprint(client_fd, "Terminated %d processes\n", nprocs);
}